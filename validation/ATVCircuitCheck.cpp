#include "Scene.h"
#include <fstream>
#include <stdexcept>
#include <cstdint>

static void require(bool condition,const char* message)
{if(!condition)throw std::runtime_error(message);}

static void saveFrame(const char* path)
{
    const int width=960,height=640;
    std::vector<unsigned char> pixels(width*height*3);
    glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    for(size_t i=0;i<pixels.size();i+=3)std::swap(pixels[i],pixels[i+2]);
    std::ofstream out(path,std::ios::binary);
    auto word=[&](uint16_t v){out.write((char*)&v,2);};auto dword=[&](uint32_t v){out.write((char*)&v,4);};
    word(0x4d42);dword(54+(uint32_t)pixels.size());dword(0);dword(54);dword(40);
    dword(width);dword(height);word(1);word(24);dword(0);dword((uint32_t)pixels.size());
    dword(2835);dword(2835);dword(0);dword(0);out.write((char*)pixels.data(),pixels.size());
    require(out.good(),"Could not save ATV preview");
}

int main()
{
    try
    {
        BeachCircuit circuit;
        for(float dt:{1/60.f,1/30.f,.05f,.1f}){
            UmbrellaAnimationState animation;
            require(animation.openAmount==1 && std::string(animation.label())=="OPEN","Umbrella does not start open");
            require(animation.press(true) && !animation.targetOpen,"I does not close umbrella");
            require(!animation.press(true) && !animation.targetOpen,"Held I repeats umbrella toggle");
            animation.press(false);
            for(int i=0;i<(int)roundf(1.5f/dt);++i)animation.update(dt);
            require(animation.openAmount==0 && std::string(animation.label())=="CLOSED","Closing duration depends on FPS");
            animation.press(true);animation.press(false);
            for(int i=0;i<(int)roundf(.5f/dt);++i)animation.update(dt);
            float before=animation.openAmount;require(before>0 && before<1,"Umbrella has no intermediate poses");
            animation.press(true);require(animation.openAmount==before,"Reversal jumps umbrella pose");animation.press(false);
            for(int i=0;i<(int)roundf(1.5f/dt);++i)animation.update(dt);
            require(animation.openAmount==0,"Reversal fails to reach closed endpoint");
            animation.press(true);animation.press(false);animation.update(2);
            require(animation.openAmount==1,"Long frame fails to complete opening");
        }
        DayCyclePanel dayPanel;dayPanel.layout(960,640);
        float sliderSpeed=.35f;bool sliderPaused=false;
        require(!dayPanel.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,500,100,sliderSpeed,sliderPaused),"Slider consumes scene clicks");
        require(dayPanel.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,36,576,sliderSpeed,sliderPaused),"Slider track does not capture click");
        require(fabsf(sliderSpeed-.05f)<.00001f,"Slider minimum is wrong");
        require(dayPanel.move(340,sliderSpeed) && fabsf(sliderSpeed-4)<.00001f,"Slider maximum is wrong");
        dayPanel.move(2000,sliderSpeed);require(sliderSpeed==4,"Drag outside slider exceeds maximum");
        require(dayPanel.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,2000,10,sliderSpeed,sliderPaused) && !dayPanel.active(),"Slider release leaks camera input");
        dayPanel.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,278,538,sliderSpeed,sliderPaused);
        require(sliderPaused,"Panel pause button does not pause");dayPanel.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,278,538,sliderSpeed,sliderPaused);
        dayPanel.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,278,538,sliderSpeed,sliderPaused);
        require(!sliderPaused,"Panel play button does not resume");dayPanel.cancel();
        dayPanel.layout(480,320);require(dayPanel.contains(36,260),"Slider does not follow window resize");
        ObjectGuideState guide;guide.layout(960,640);guide.observe(GUIDE_LIGHTHOUSE);
        require(guide.visible&&guide.selected==GUIDE_LIGHTHOUSE,"Object guide does not select focused object");
        guide.selected=GUIDE_UMBRELLA;guide.observe(GUIDE_LIGHTHOUSE);
        require(guide.selected==GUIDE_UMBRELLA,"Guide overrides manual section on unchanged context");
        guide.observe(GUIDE_WALKING);require(guide.selected==GUIDE_WALKING,"Walking context is not automatic");
        require(!guide.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,800,600),"Guide captures scene click");
        require(guide.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,30,guide.top+48),"Guide header cannot be expanded");
        require(guide.active()&&guide.mouse(GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,900,600)&&!guide.active(),"Guide release outside panel leaks input");
        guide.layout(480,320);guide.observe(GUIDE_ATV);
        require(guide.top+guide.height<320-18-102*guide.scale,"Guide overlaps day slider");
        require(guide.wheel(30,60,-4)&&guide.scroll>0,"Guide wheel does not scroll");
        require(!guide.wheel(470,60,1),"Guide consumes camera zoom outside panel");
        guide.toggle();guide.layout(480,320);require(!guide.visible&&guide.height<=32*guide.scale,"Guide does not collapse");
        guide.toggle();guide.layout(960,640);
        for(int section=0;section<9;++section){guide.observe(section);require(guide.selected==section&&!guide.rows.empty(),"Missing object instruction section");}
        require(LighthouseLanternDimensions::opticY>LighthouseLanternDimensions::bottom+.4f&&
                LighthouseLanternDimensions::opticY<LighthouseLanternDimensions::top-.4f,
                "Optic does not fit inside the lantern window height");
        for(int bay=0;bay<12;++bay){
            auto pane=lighthouseWindowTransform(glm::mat4(1),bay,LighthouseLanternDimensions::opticY);
            auto centre=glm::vec3(pane[3]);
            require(fabsf(glm::length(glm::vec2(centre.x,centre.z))-LighthouseLanternDimensions::radius*cosf(glm::pi<float>()/12))<.0001f,
                    "Lantern window is not on the twelve-sided enclosure");
            require(fabsf(glm::dot(glm::vec3(pane[0]),glm::vec3(pane[2])))<.0001f,
                    "Window tangent and outward normal are not perpendicular");
        }
        LighthouseBeamState beacon;
        beacon.update(1);require(fabsf(beacon.heading-55)<.001f,"Beacon sweep speed is wrong");
        beacon.paused=true;beacon.update(10);require(fabsf(beacon.heading-55)<.001f,"Paused beam still moves");
        beacon.update(2,-1);require(fabsf(beacon.heading-355)<.001f && beacon.paused,"Manual aim does not wrap/pause");
        beacon.update(100,0,1,1);require(beacon.speed==90 && beacon.elevation==5,"Beam upper limits fail");
        beacon.update(100,0,-1,-1);require(beacon.speed==5 && beacon.elevation==-5,"Beam lower limits fail");
        beacon=LighthouseBeamState();beacon.update(1.0f/60);
        LighthouseBeamState fine;for(int i=0;i<10;++i)fine.update(1.0f/600);
        require(fabsf(beacon.heading-fine.heading)<.0001f,"Beam motion depends on frame rate");
        auto beachChairs=safeBeachChairPlacements();
        require(beachChairs.size()==15,"Furniture was removed instead of fixing its placement");
        for(const auto& chair:beachChairs)
        {
            require(getIslandSignedDistance(chair.x,chair.z)<12.2f,"Chair was moved into the forest instead of the beach");
            glm::mat4 m=beachChairTransform(chair);
            for(float x:{-.70f,0.0f,.70f})for(float z:{-1.35f,0.0f,.85f})
            {
                glm::vec3 p(m*glm::vec4(x,0,z,1));
                require(getIslandSignedDistance(p.x,p.z)>10,"Lounger footprint reaches the sea or driving lane");
                require(getCoastalTerrainElevation(p.x,p.z)>.22f+.35f,"Highest tide/swash reaches a lounger");
            }
            glm::vec3 nearest=circuit.sample(circuit.nearestDistance(glm::vec3(chair.x,0,chair.z)));
            require(glm::length(glm::vec2(nearest.x-chair.x,nearest.z-chair.z))>3.0f,"Moved lounger blocks ATV circuit");
        }
        require(fabsf(coastalTideLevel(12.42))<.00001f,"Tide does not complete its semidiurnal cycle");
        require(tidalShoreInset(.22f)>tidalShoreInset(-.22f),"High tide retreats instead of advancing");
        for(double hour=0;hour<24;hour+=.001)
            require(fabsf(coastalTideLevel(hour+.001)-coastalTideLevel(hour))<.0002f,"Tide changes abruptly");
        glm::mat4 station=beachChargingStationTransform(circuit);
        glm::vec3 stationPosition(station[3]);
        BeachATVState chargingATV;
        chargingATV.pos=glm::vec3(station*glm::vec4(-.45f,0,0,1));
        chargingATV.pos.y=getCoastalTerrainElevation(chargingATV.pos.x,chargingATV.pos.z);
        chargingATV.heading=glm::degrees(atan2f(station[2].x,station[2].z));
        require(canConnectATVCharger(chargingATV,circuit),"ATV cannot connect from parking space");
        chargingATV.speed=.2f;require(!canConnectATVCharger(chargingATV,circuit),"Moving ATV can connect");chargingATV.speed=0;
        chargingATV.touring=true;require(!canConnectATVCharger(chargingATV,circuit),"Touring ATV can connect");chargingATV.touring=false;
        auto parked=chargingATV.pos;chargingATV.pos=circuit.sample(0);require(!canConnectATVCharger(chargingATV,circuit),"Distant ATV can connect");chargingATV.pos=parked;
        for(float dt:{1/60.f,1/30.f,.05f,.1f}){
            ATVBatteryState battery;battery.percent=0;battery.charging=true;
            require(battery.press(true) && !battery.press(true),"Held Q repeats charging toggle");battery.press(false);require(battery.press(true),"Q cannot be pressed again");
            BeachATVState plugged=chargingATV;
            for(int i=0;i<(int)roundf(15/dt);++i)updatePoweredBeachATV(plugged,battery,circuit,1,1,dt);
            require(fabsf(battery.percent-50)<.01f,"Charging speed depends on FPS");
            require(glm::distance(plugged.pos,parked)<.00001f && plugged.speed==0 && !plugged.touring,"Connected ATV can move");
            battery.advance(30);require(battery.percent==100 && battery.charging,"Full charge exceeds limit or disconnects unexpectedly");
            battery.charging=false;auto before=plugged.pos;
            for(int i=0;i<(int)roundf(.5f/dt);++i)updatePoweredBeachATV(plugged,battery,circuit,1,0,dt);
            require(glm::distance(plugged.pos,before)>.1f && battery.percent<100,"Disconnect fails to restore driving/drain");
        }
        ATVBatteryState empty;empty.percent=0;BeachATVState powerless=chargingATV;
        for(int i=0;i<120;++i)updatePoweredBeachATV(powerless,empty,circuit,1,0,1/60.f,false);
        require(glm::distance(powerless.pos,parked)<.001f && powerless.speed==0,"Empty battery produces motor power");
        ATVBatteryState consumption;consumption.useDistance(400);require(fabsf(consumption.percent-50)<.001f,"Distance discharge wrong");
        consumption.useDistance(10000);require(consumption.percent==0,"Battery charge becomes negative");
        for(float x:{-1.8f,1.8f})for(float z:{-2.3f,2.3f})
        {
            glm::vec3 corner(station*glm::vec4(x,0,z,1));
            require(getIslandSignedDistance(corner.x,corner.z)>3,"Charger foundation reaches wet sand");
        }
        for(int i=0;i<4096;++i)
        {
            glm::vec3 p=circuit.sample(circuit.length*i/4096);
            glm::vec3 local(glm::inverse(station)*glm::vec4(p,1));
            require(fabsf(local.x)>3.1f || fabsf(local.z)>3.65f,"Charging bay obstructs the ATV lane");
        }
        float smallest=1e9f,largest=-1e9f;
        for(int i=0;i<4096;++i)
        {
            float heading;glm::vec3 p=circuit.sample(circuit.length*i/4096,&heading);
            float d=getIslandSignedDistance(p.x,p.z);smallest=glm::min(d,smallest);largest=glm::max(d,largest);
            require(atvFootprintIsDry(p,heading),"Full ATV footprint cannot complete the coastal circuit");
            require(p.y>.22f+.10f,"High tide floods the ATV circuit");
            glm::vec3 eye=glm::vec3(beachATVTransform(BeachATVState{p,heading})*glm::vec4(0,2.1f,.18f,1));
            require(eye.y>getCoastalTerrainElevation(eye.x,eye.z)+1.6f,"Rider camera penetrates terrain");
        }
        for(float dt:{1/60.0f,1/30.0f,0.05f,.1f})
        {
            BeachATVState state;state.pos=circuit.sample(circuit.length*.37f);
            glm::vec3 start=state.pos;startBeachTour(state,circuit);
            int iterations=0;
            while(state.touring && iterations++<20000)
            {
                updateBeachATV(state,circuit,0,0,dt);
                require(atvFootprintIsDry(state.pos,state.heading),"Automatic tour left the dry beach");
            }
            require(!state.touring && state.completedTours==1,"Tour did not finish exactly one lap");
            require(glm::distance(start,state.pos)<.08f,"Tour did not return to its starting position");
            require(state.speed==0,"ATV did not stop after the round trip");
        }
        // The current scene's south beach furniture must not block the route.
        for(glm::vec3 chair:{glm::vec3(0,0,39),glm::vec3(-32,0,34),glm::vec3(-24,0,39),
             glm::vec3(-16,0,42),glm::vec3(-8,0,44),glm::vec3(9,0,44),glm::vec3(18,0,41),glm::vec3(28,0,36)})
        {
            glm::vec3 nearest=circuit.sample(circuit.nearestDistance(chair));
            require(glm::length(glm::vec2(nearest.x-chair.x,nearest.z-chair.z))>2.5f,"Lounger obstructs the full-island tour");
        }
        BeachATVState manual;manual.pos=circuit.sample(0,&manual.heading);
        float oldHeading=manual.heading;
        for(int i=0;i<30;++i)updateBeachATV(manual,circuit,1,1,1/60.0f);
        require(manual.heading<oldHeading,"D/right steering turns the wrong way");
        auto facing=[](float heading){float a=glm::radians(heading);return glm::vec3(sinf(a),0,cosf(a));};
        require(glm::dot(facing(manual.heading),glm::cross(facing(oldHeading),glm::vec3(0,1,0)))>0,
                "D steering is not right in the rider/chase frame");
        require(fabsf(manual.wheelSpin)>1,"Tyres do not rotate while driving");
        // A grounded ATV steers its wheels at rest without yawing its chassis.
        BeachATVState stationarySteering;stationarySteering.pos=circuit.sample(0,&stationarySteering.heading);
        stationarySteering.yawRate=.6f;
        float stoppedHeading=stationarySteering.heading;glm::vec3 stoppedPosition=stationarySteering.pos;
        updateBeachATV(stationarySteering,circuit,0,1,.2f,true);
        require(fabsf(stationarySteering.heading-stoppedHeading)<.0001f &&
                glm::length(glm::vec2(stationarySteering.pos.x-stoppedPosition.x,
                                      stationarySteering.pos.z-stoppedPosition.z))<.0001f && stationarySteering.yawRate==0,
                "Stopped ATV retains boat-like yaw momentum");
        require(stationarySteering.steering<0,"D does not turn stopped front wheels right");
        BeachATVState reverseTurn;reverseTurn.pos=circuit.sample(0,&reverseTurn.heading);
        reverseTurn.speed=-1;reverseTurn.steering=-15;reverseTurn.parked=false;
        float reverseHeading=reverseTurn.heading;
        updateBeachATV(reverseTurn,circuit,-1,.5f,.1f);
        require(reverseTurn.heading>reverseHeading,"Reverse ATV steering has forward yaw direction");
        startBeachTour(manual,circuit);updateBeachATV(manual,circuit,1,0,1/60.0f);
        require(!manual.touring,"Manual control did not cancel the guided tour");
        // Slow rendering must not change distance, speed or steering response.
        BeachATVState reference;
        for(float dt:{1/60.0f,1/30.0f,.05f,.1f})
        {
            BeachATVState drive;drive.pos=circuit.sample(0,&drive.heading);
            for(int i=0;i<(int)roundf(1.0f/dt);++i)updateBeachATV(drive,circuit,1,0,dt);
            if(dt==1/60.0f)reference=drive;
            require(glm::distance(drive.pos,reference.pos)<.025f,"Manual distance changes with frame rate");
            require(fabsf(drive.speed-reference.speed)<.025f,"Manual speed changes with frame rate");
            float movingSpeed=drive.speed;
            updateBeachATV(drive,circuit,0,0,.1f);
            require(drive.speed>movingSpeed*.85f,"Coasting stops like a toy");
            for(int i=0;i<20;++i)updateBeachATV(drive,circuit,0,0,.1f,true);
            require(fabsf(drive.speed)<.01f,"Separate brake does not stop the ATV");
            for(int i=0;i<10;++i)updateBeachATV(drive,circuit,-1,0,.1f);
            require(drive.speed<-.1f,"Hold S does not select reverse");
        }
        BeachATVState turnReference;
        for(float dt:{1/60.0f,.1f})
        {
            BeachATVState drive;drive.pos=circuit.sample(0,&drive.heading);
            for(int i=0;i<(int)roundf(2.0f/dt);++i)
            {
                updateBeachATV(drive,circuit,1,.3f,dt);
                require(atvFootprintIsDry(drive.pos,drive.heading),"Manual steering drove into the water");
            }
            if(dt==1/60.0f)turnReference=drive;
            require(glm::distance(drive.pos,turnReference.pos)<.025f,"Cornering changes at low FPS");
            require(fabsf(drive.heading-turnReference.heading)<.05f,"Steering yaw changes at low FPS");
            require(fabsf(drive.bodyPitch)<=4.1f && fabsf(drive.bodyRoll)<=3.1f,"Suspension response is unstable");
        }

        require(glfwInit()!=0,"GLFW initialization failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
        glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
        glfwWindowHint(GLFW_SAMPLES,4);
        GLFWwindow* window=glfwCreateWindow(960,640,"ATV test",nullptr,nullptr);
        require(window!=nullptr,"OpenGL context failed");glfwMakeContextCurrent(window);
        require(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)!=0,"GLAD failed");
        dayPanel.init();
        Shader objects("default.vert","default.frag"),water("water.vert","water.frag");
        CubeMesh cube;SphereMesh sphere;FrustumMesh cylinder(.5f,.5f,32);
        LighthouseSite lighthouseSite;
        FrustumMesh tower(.5f,.34f,40),roof(.55f,0,40),beam(.5f,LighthouseLanternDimensions::beamTipRadius,48,false);
        for(int ring=0;ring<=40;++ring)for(int i=0;i<360;++i)
        {
            float a=glm::radians((float)i),r=LighthouseSite::radius*ring/40;
            require(lighthouseSite.baseHeight()>getCoastalTerrainElevation(-6+r*cosf(a),-110+r*sinf(a)),
                    "Lighthouse platform cuts through the summit");
        }
        CanopyGoreMesh canopy;
        float canopyVertices[16*8];glBindBuffer(GL_ARRAY_BUFFER,canopy.VBO);
        glGetBufferSubData(GL_ARRAY_BUFFER,0,sizeof(canopyVertices),canopyVertices);
        require(canopyVertices[6*8+2]-canopyVertices[14*8+2]>.007f,"Umbrella valance still has coplanar inner/outer faces");
        // Sample both skins and every triangle interior against the pole,
        // actual terrain and a conservative seated robot/furniture envelope.
        auto mainChair=safeBeachChairPlacements()[0];auto chairMatrix=beachChairTransform(mainChair);
        auto inverseChair=glm::inverse(chairMatrix);
        auto inverseBack=glm::inverse(glm::rotate(glm::translate(glm::mat4(1),glm::vec3(0,.49f,-.66f)),glm::radians(-28.f),glm::vec3(1,0,0)));
        glm::vec3 umbrellaBase(mainChair.x-1.6f,0,mainChair.z-1.2f);
        umbrellaBase.y=getCoastalTerrainElevation(umbrellaBase.x,umbrellaBase.z);
        GLuint goreIndices[36];glBindBuffer(GL_ARRAY_BUFFER,canopy.EBO);
        glGetBufferSubData(GL_ARRAY_BUFFER,0,sizeof(goreIndices),goreIndices);
        for(float tilt:{0.f,14.f,40.4f})for(int stage=0;stage<=20;++stage){
            float opening=stage/20.f;
            auto node=glm::rotate(glm::translate(glm::mat4(1),umbrellaBase+glm::vec3(0,1,0)),glm::radians(tilt),glm::vec3(0,0,1));
            auto crown=glm::translate(node,{0,2.4f,0});
            for(int sector=0;sector<12;++sector){auto panel=umbrellaPanelTransform(opening,sector);
                for(int triangle=0;triangle<12;++triangle)for(int i=0;i<=4;++i)for(int j=0;j<=4-i;++j){
                    glm::vec3 v[3];for(int k=0;k<3;++k){int index=goreIndices[triangle*3+k]*8;v[k]={canopyVertices[index],canopyVertices[index+1],canopyVertices[index+2]};}
                    glm::vec4 local=panel*glm::vec4(v[0]*(1-(i+j)/4.f)+v[1]*(i/4.f)+v[2]*(j/4.f),1);
                    require(glm::length(glm::vec2(local.x,local.z))>.05f,"Folding canopy intersects pole");
                    glm::vec3 world(crown*local),chairLocal(inverseChair*glm::vec4(world,1));
                    require(world.y>getCoastalTerrainElevation(world.x,world.z)+.06f,"Folding canopy intersects terrain");
                    bool chairClear=fabsf(chairLocal.x)>.65f || fabsf(chairLocal.z)>.85f || chairLocal.y>1.2f;
                    bool robotClear=fabsf(chairLocal.x)>.48f || fabsf(chairLocal.z-.36f)>.32f || chairLocal.y>1.94f;
                    glm::vec3 backLocal(inverseBack*glm::vec4(chairLocal,1));
                    chairClear &= fabsf(backLocal.x)>.57f || backLocal.y<-.02f || backLocal.y>1.08f || fabsf(backLocal.z)>.06f;
                    if(!chairClear || !robotClear)std::cerr<<"Umbrella clearance: tilt "<<tilt<<" opening "<<opening<<" chair-local "<<chairLocal.x<<","<<chairLocal.y<<","<<chairLocal.z<<"\n";
                    require(chairClear && robotClear,"Folding canopy intersects seated robot/chair");
                }
            }
        }
        IslandTerrainMesh terrain;WaterGridMesh ocean;Skybox sky;
        TextureManager textures;textures.init();
        IslandNightLights nightLights(circuit);
        for(Shader* lampShader:{&objects,&water}){
            auto body=beachATVBodyTransform(glm::mat4(1),7,-4);
            nightLights.bind(*lampShader,22,&body);
            GLint count=0;glGetUniformiv(lampShader->ID,lampShader->location("nightLightCount"),&count);
            require(count==(int)nightLights.count()+2,"ATV headlights are missing from the light pass");
            glm::vec4 position,direction;
            std::string slot=std::to_string(nightLights.count());
            glGetUniformfv(lampShader->ID,lampShader->location(("nightLightPositions["+slot+"]").c_str()),glm::value_ptr(position));
            glGetUniformfv(lampShader->ID,lampShader->location(("nightLightDirections["+slot+"]").c_str()),glm::value_ptr(direction));
            require(glm::length(glm::vec3(position)-glm::vec3(body*glm::vec4(-.38f,.95f,1.08f,1)))<.0001f,"Headlight source separates from its ATV lens");
            require(direction.w==1&&fabsf(glm::length(glm::vec3(direction))-1)<.0001f,"Headlight cone has an invalid direction");
            nightLights.bind(*lampShader,12,&body);float intensity=1;
            glGetUniformfv(lampShader->ID,lampShader->location("nightLightIntensity"),&intensity);
            require(intensity==0,"ATV lights remain on during daylight");
            nightLights.bind(*lampShader,22);
        }
        IslandVegetation vegetation(circuit);
        // Batch capture uses the original geometry/material definitions.
        IslandForestBatches forest;forest.init(objects,terrain,cube,sphere);forest.update(objects,cube,sphere,.8f);
        ForestCapture expected;g_forestCapture=&expected;
        drawIslandTerrainBackdrop(objects,terrain,cube,sphere);
        drawBackgroundPalmForest(objects,cube,sphere,.8f);g_forestCapture=nullptr;
        require(forest.instanceCount()==expected.cubes.size()+expected.spheres.size()+expected.leaves.size(),"Forest batching loses primitives");
        require(!expected.leaves.empty(),"Textured palm leaf instances missing");
        auto roots=forest.roots();require(roots.size()==expected.roots.size(),"Forest root counts differ");
        for(size_t i=0;i<roots.size();++i)require(glm::distance(roots[i],expected.roots[i])<.00001f,"Forest root position changed");
        GLint modelLocation=objects.location("model");size_t cacheSize=objects.uniformLocations.size();
        for(int i=0;i<100;++i)require(objects.location("model")==modelLocation,"Uniform cache unstable");
        require(objects.uniformLocations.size()==cacheSize,"Uniform cache grows on reuse");
        ForestInstance inside{glm::mat4(1),{1,1,1},{1,0,0}},outside=inside,edge=inside;
        outside.model[3]=glm::vec4(5,0,0,1);edge.model[3]=glm::vec4(1.2f,0,0,1);
        require(forestInstanceVisible(inside,glm::mat4(1)) && !forestInstanceVisible(outside,glm::mat4(1)) && forestInstanceVisible(edge,glm::mat4(1)),"Conservative forest culling wrong");
        glm::mat4 house=glm::rotate(glm::translate(glm::mat4(1),glm::vec3(-24,getCoastalTerrainElevation(-24,10),10)),glm::radians(15.f),glm::vec3(0,1,0));
        for(float x:{-3.4f,-1.7f,0.f,1.7f,3.4f})for(float z:{-1.8f,1.8f}){
            auto support=terrainSupportRange(house,x,z,.23f,.23f);
            for(int i=0;i<=4;++i)for(int j=0;j<=4;++j){auto p=house*glm::vec4(x-.23f+.46f*i/4,0,z-.23f+.46f*j/4,1);
                float y=getCoastalTerrainElevation(p.x,p.z)-house[3].y;
                require(support.x<y && support.y>y && support.y<1.80f,"House footing unsupported");}
        }
        for(float angle:{0.f,90.f,180.f,270.f})for(bool circle:{false,true}){
            auto transform=glm::rotate(glm::mat4(1),glm::radians(35.f),glm::vec3(0,1,0));
            g_atvObstacles={circle?SolidObstacle::circle({0,0,0},.2f):SolidObstacle::box(transform,{.3f,.6f})};
            glm::vec3 f(sinf(glm::radians(angle)),0,cosf(glm::radians(angle)));
            require(!atvSweepClear(-f*4.f,angle,f*4.f,angle),"ATV sweep tunnels through solid object");
            require(atvSweepClear(-f*4.f,angle,-f*5.f,angle),"ATV cannot move away from object");
        }
        g_atvObstacles={SolidObstacle::circle({1.5f,0,0},.1f)};
        require(atvSweepClear({0,0,0},179.f,{0,0,0},-179.f),"Heading wrap creates a false full-turn collision");
        for(float dt:{1/60.f,1/30.f,.05f,.1f}){
            BeachATVState ride;ride.pos=circuit.sample(0,&ride.heading);glm::vec3 start=ride.pos;
            glm::vec3 f(sinf(glm::radians(ride.heading)),0,cosf(glm::radians(ride.heading)));
            g_atvObstacles={SolidObstacle::circle(start+f*4.f,.2f)};
            bool hit=false;for(int i=0;i<600 && !hit;++i){updateBeachATV(ride,circuit,1,0,dt);hit=ride.blocked;require(atvSolidsClear(ride.pos,ride.heading),"ATV penetrates obstacle");}
            require(hit && ride.speed==0,"ATV does not stop on contact");auto contact=ride.pos;
            for(int i=0;i<(int)(2/dt);++i)updateBeachATV(ride,circuit,-1,0,dt);
            require(glm::distance(contact,ride.pos)>.25f && ride.speed<0,"ATV cannot reverse away after contact");
        }
        g_atvObstacles=islandSolidObstacles(circuit,safeBeachChairPlacements(),nightLights,roots,vegetation.woodyRoots());
        require(beachCircuitSolidsClear(circuit),"Solid props obstruct full beach circuit");
        require(beachRobotPositionClear(BeachRobotState().pos,g_atvObstacles),"Walking robot starts intersecting a solid prop");
        for(float d=0;d<circuit.length;d+=1)
            require(beachRobotPositionClear(circuit.sample(d),g_atvObstacles),"Walking path is obstructed by scene props");
        for(size_t index=0;index<g_atvObstacles.size();++index)if(atvTouchesObstacle(chargingATV.pos,chargingATV.heading,g_atvObstacles[index])){
            auto local=glm::inverse(station)*glm::vec4(g_atvObstacles[index].center.x,0,g_atvObstacles[index].center.y,1);
            std::cerr<<"Parking obstacle "<<index<<" bay local "<<local.x<<","<<local.z<<"\n";}
        require(canConnectATVCharger(chargingATV,circuit),"Solid collision world blocks charger parking position");
        for(float dt:{1/60.f,1/30.f,.05f,.1f}){BeachATVState tour;tour.pos=circuit.sample(0);startBeachTour(tour,circuit);
            int iterations=0;while(tour.touring && ++iterations<20000){updateBeachATV(tour,circuit,0,0,dt);require(!tour.blocked,"Guided ATV hits a prop");}
            require(tour.completedTours==1,"Obstacle-aware tour fails to complete");}
        BeachATVState stopped;stopped.pos=circuit.sample(0);startBeachTour(stopped,circuit);
        g_atvObstacles={SolidObstacle::circle(circuit.sample(5),.3f)};
        for(int i=0;i<600 && stopped.touring;++i)updateBeachATV(stopped,circuit,0,0,1/60.f);
        require(stopped.blocked && stopped.completedTours==0 && stopped.speed==0,"Obstructed tour does not stop safely");
        g_atvObstacles.clear();
        require(islandNightIntensity(12)==0 && islandNightIntensity(22)==1,"Night lamps do not follow the day cycle");
        for(float hour=0;hour<24;hour+=.01f)
            require(fabsf(islandNightIntensity(hour+.01f)-islandNightIntensity(hour))<.012f,"Night lights switch abruptly");
        require(vegetation.palmCount()>100 && vegetation.grassCount()>1400 && vegetation.understoryCount()>120,"Added vegetation is too sparse");
        require(vegetation.broadleafCount()>60 && vegetation.fernCount()>100 && vegetation.flowerCount()>40 && vegetation.fanPalmCount()>25,"New tropical species are missing");
        for(glm::vec3 p:vegetation.roots())
        {
            glm::vec3 bayLocal(glm::inverse(station)*glm::vec4(p,1));
            require(fabsf(bayLocal.x)>=6 || fabsf(bayLocal.z)>=6,"Vegetation obstructs the charging bay");
            require(getIslandSignedDistance(p.x,p.z)>=22,"Vegetation blocks the beach driving lane");
            require(fabsf(p.y-getCoastalTerrainElevation(p.x,p.z))<.001f,"Vegetation floats above terrain");
            require(glm::length(glm::vec2(p.x+6,p.z+110))>=17,"Vegetation obstructs lighthouse access");
        }
        const auto& woody=vegetation.woodyRoots();
        for(size_t i=0;i<woody.size();++i)for(size_t j=i+1;j<woody.size();++j)
            require(glm::length(glm::vec2(woody[i].x-woody[j].x,woody[i].z-woody[j].z))>=4.499f,"Large trees intersect each other");
        for(glm::vec4 lamp:nightLights.locations())
        {
            glm::vec3 route=circuit.sample(circuit.nearestDistance(glm::vec3(lamp)));
            require(glm::length(glm::vec2(lamp.x-route.x,lamp.z-route.z))>2.5f,"Lamp post obstructs ATV circuit");
        }
        SunShadows shadows;shadows.init();
        ATVTracks tracks;tracks.init();
        BeachATVState trail;
        trail.pos=circuit.sample(-18,&trail.heading);tracks.update(trail);
        require(tracks.stampCount()==0,"Stationary ATV leaves tracks");
        tracks.update(trail);require(tracks.stampCount()==0,"Stationary ATV leaves tracks");
        for(int i=1;i<=200;++i)
        {
            trail.pos=circuit.sample(-18+i*.10f,&trail.heading);tracks.update(trail);
        }
        require(tracks.stampCount()>400,"Moving ATV does not leave four tyre trails");
        size_t marks=tracks.stampCount();tracks.update(trail);
        require(tracks.stampCount()==marks,"Tracks accumulate while parked");
        for(int i=1;i<=30;++i)
        {trail.pos=circuit.sample(2-i*.10f,&trail.heading);tracks.update(trail);}
        require(tracks.stampCount()>marks,"Reversing does not leave tracks");
        marks=tracks.stampCount();trail.pos=circuit.sample(200,&trail.heading);tracks.update(trail);
        require(tracks.stampCount()==marks,"Teleport produces a false tyre trail");
        glEnable(GL_DEPTH_TEST);glViewport(0,0,960,640);
        BeachATVState state;state.pos=circuit.sample(0,&state.heading);
        glm::vec3 light=glm::normalize(glm::vec3(-.4f,.8f,.3f));
        double litGround=0;
        for(int shot=0;shot<8;++shot)
        {
            g_sceneSeaLevel=shot==4?-.22f:shot==5?.22f:0;
            state.pos=circuit.sample(shot==2?circuit.length*.47f:0,&state.heading);
            glm::mat4 vehicle=beachATVTransform(state);
            glm::vec3 eye;
            glm::mat4 view;
            if(shot>=6)
            {
                glm::vec3 lamp(nightLights.locations()[0]);
                eye=lamp+glm::vec3(6,2,7);
                view=glm::lookAt(eye,lamp-glm::vec3(0,1.9f,0),glm::vec3(0,1,0));
            }
            else if(shot==3)
            {
                eye=glm::vec3(station*glm::vec4(-7,4,6,1));
                view=glm::lookAt(eye,stationPosition+glm::vec3(0,1.2f,0),glm::vec3(0,1,0));
            }
            else if(shot==2)
            {
                eye=glm::vec3(vehicle*glm::vec4(0,2.1f,.18f,1));
                glm::vec3 direction=glm::vec3(vehicle*glm::vec4(0,0,1,0));
                view=glm::lookAt(eye,eye+direction,glm::vec3(0,1,0));
            }
            else
            {
                eye=glm::vec3(vehicle*glm::vec4(shot==0?3.3f:-3.5f,shot==0?2.5f:2.8f,shot==0?4.5f:-5.0f,1));
                view=glm::lookAt(eye,state.pos+glm::vec3(0,1.1f,0),glm::vec3(0,1,0));
            }
            glm::mat4 projection=glm::perspective(glm::radians(shot==2?60.0f:42.0f),1.5f,.1f,2000.0f);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(view,projection,shot>=6?22:12,light);
            objects.Activate();
            glUniformMatrix4fv(glGetUniformLocation(objects.ID,"view"),1,GL_FALSE,glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(objects.ID,"proj"),1,GL_FALSE,glm::value_ptr(projection));
            glUniform3fv(glGetUniformLocation(objects.ID,"viewPos"),1,glm::value_ptr(eye));
            glUniform3fv(glGetUniformLocation(objects.ID,"lightDir"),1,glm::value_ptr(light));
            glUniform3f(glGetUniformLocation(objects.ID,"lightColor"),shot>=6?.18f:1,shot>=6?.24f:.98f,shot>=6?.45f:.92f);
            glUniform1f(glGetUniformLocation(objects.ID,"ambientStrength"),shot>=6?.15f:.42f);
            nightLights.bind(objects,shot>=6?22:12);nightLights.bind(water,shot>=6?22:12);objects.Activate();
            if(shot==7){glUniform1f(glGetUniformLocation(objects.ID,"nightLightIntensity"),0);water.Activate();glUniform1f(glGetUniformLocation(water.ID,"nightLightIntensity"),0);objects.Activate();}
            glUniform1f(glGetUniformLocation(objects.ID,"objectAlpha"),1);
            auto opaque=[&]() {
                drawIslandTerrainBackdrop(objects,terrain,cube,sphere,&textures);
                vegetation.draw(objects,7.5f);
                nightLights.draw(objects,cube,sphere,shot>=6?22:12);
                drawBeach4WD(objects,cube,sphere,cylinder,vehicle,35,shot==0?12.0f:0,0);
                drawATVChargingStation(objects,cube,cylinder,station);
            };
            if(shot<6){shadows.begin(objects,light,45,shot==3?stationPosition:state.pos);opaque();shadows.end(objects,960,640,view,projection);}
            shadows.bind(objects,shot<6);opaque();
            tracks.draw(objects);
            shadows.bind(water,shot<6);
            drawOceanWaves(water,ocean,view,projection,eye,light,glm::vec3(1,.98f,.92f),.42f,7.5f,&textures);
            glFinish();require(glGetError()==GL_NO_ERROR,"ATV rendering produced an OpenGL error");
            saveFrame(shot==0?"build/validation/atv-front.bmp":shot==1?"build/validation/atv-chase.bmp":shot==2?"build/validation/atv-rider.bmp":shot==3?"build/validation/atv-charger.bmp":shot==4?"build/validation/environment-low-tide.bmp":shot==5?"build/validation/environment-high-tide.bmp":shot==6?"build/validation/island-night-lights.bmp":"build/validation/island-night-lights-off.bmp");
            if(shot>=6)
            {
                std::vector<unsigned char> pixels(960*640*3);glReadPixels(0,0,960,640,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
                double brightness=0;for(unsigned char value:pixels)brightness+=value;
                if(shot==6)litGround=brightness;
                else require(litGround>brightness*1.15,"Night lamps glow but do not illuminate the scene");
            }
            if(shot==0)
            {
                std::vector<unsigned char> previous;
                for(int mode=0;mode<4;++mode)
                {
                    objects.Activate();glUniform1i(glGetUniformLocation(objects.ID,"forceShadingMode"),mode);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);opaque();
                    glFinish();std::vector<unsigned char> pixels(960*640*3);
                    glReadPixels(0,0,960,640,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
                    if(mode>0)require(pixels!=previous,"Shading switch does not affect rendered objects");
                    previous=pixels;
                    saveFrame(mode==0?"build/validation/shading-auto.bmp":mode==1?"build/validation/shading-flat.bmp":mode==2?"build/validation/shading-gouraud.bmp":"build/validation/shading-phong.bmp");
                }
                glUniform1i(glGetUniformLocation(objects.ID,"forceShadingMode"),0);
            }
        }
        for(float level:{20.f,100.f}){
            auto vehicle=beachATVTransform(chargingATV);
            glm::vec3 eye(station*glm::vec4(level<100?6.f:-6.f,4,level<100?-6.f:6.f,1)),target=stationPosition+glm::vec3(0,1.2f,0);
            auto view=glm::lookAt(eye,target,glm::vec3(0,1,0));
            auto projection=glm::perspective(glm::radians(42.f),1.5f,.1f,2000.f);
            objects.Activate();glUniform3fv(objects.location("viewPos"),1,glm::value_ptr(eye));
            glUniform3fv(objects.location("lightDir"),1,glm::value_ptr(light));
            glUniform3f(objects.location("lightColor"),1,.98f,.92f);glUniform1f(objects.location("ambientStrength"),.42f);
            nightLights.bind(objects,12);objects.Activate();
            auto opaque=[&](){drawIslandTerrainBackdrop(objects,terrain,cube,sphere,&textures);
                drawBeach4WD(objects,cube,sphere,cylinder,vehicle);
                drawATVChargingStation(objects,cube,cylinder,station,true,level,glm::vec3(vehicle*glm::vec4(.48f,.9f,-.5f,1)));};
            shadows.begin(objects,light,45,stationPosition);opaque();shadows.end(objects,960,640,view,projection);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(view,projection,12,light);shadows.bind(objects,true);opaque();
            glFinish();require(glGetError()==GL_NO_ERROR,"Connected charging station rendering failed");
            saveFrame(level<100?"build/validation/charging-connected.bmp":"build/validation/charging-full.bmp");
        }
        for(int shot=0;shot<3;++shot)
        {
            g_sceneSeaLevel=.22f;
            auto chair=beachChairs[shot==0?10:0];glm::mat4 chairTransform=beachChairTransform(chair);
            glm::vec3 center(chairTransform[3]);
            glm::vec3 umbrellaPosition(beachChairs[0].x-1.6f,0,beachChairs[0].z-1.2f);
            umbrellaPosition.y=getCoastalTerrainElevation(umbrellaPosition.x,umbrellaPosition.z);
            glm::mat4 umbrella=glm::translate(glm::mat4(1),umbrellaPosition);
            glm::vec3 target=shot==0?center+glm::vec3(0,.8f,0):umbrellaPosition+glm::vec3(0,2.8f,0);
            glm::vec3 eye=target+(shot==0?glm::vec3(5,2,5):shot==1?glm::vec3(3,1.8f,3):glm::vec3(2,-1,2));
            glm::mat4 view=glm::lookAt(eye,target,glm::vec3(0,1,0));
            glm::mat4 projection=glm::perspective(glm::radians(45.0f),1.5f,.1f,2000.0f);
            objects.Activate();
            glUniform3fv(glGetUniformLocation(objects.ID,"viewPos"),1,glm::value_ptr(eye));
            glUniform3fv(glGetUniformLocation(objects.ID,"lightDir"),1,glm::value_ptr(light));
            glUniform3f(glGetUniformLocation(objects.ID,"lightColor"),1,.98f,.92f);
            glUniform1f(glGetUniformLocation(objects.ID,"ambientStrength"),.42f);
            nightLights.bind(objects,12);nightLights.bind(water,12);objects.Activate();
            auto furniture=[&]() {
                drawIslandTerrainBackdrop(objects,terrain,cube,sphere,&textures);
                for(const auto& p:beachChairs)drawBeachChair(objects,cube,beachChairTransform(p));
                GLboolean prior=glIsEnabled(GL_CULL_FACE);
                drawUmbrella(objects,cube,sphere,canopy,umbrella,8,3.4f,1);
                require(glIsEnabled(GL_CULL_FACE)==prior,"Umbrella leaks face-culling state");
            };
            shadows.begin(objects,light,40,target);furniture();shadows.end(objects,960,640,view,projection);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(view,projection,12,light);
            shadows.bind(objects,true);furniture();
            shadows.bind(water,true);drawOceanWaves(water,ocean,view,projection,eye,light,glm::vec3(1,.98f,.92f),.42f,7.5f,&textures);
            glFinish();require(glGetError()==GL_NO_ERROR,"Furniture rendering failed");
            saveFrame(shot==0?"build/validation/chairs-high-tide.bmp":shot==1?"build/validation/umbrella-rim.bmp":"build/validation/umbrella-underneath.bmp");
        }
        // Open / half-folded / closed, above and below, at normal and maximum tilt.
        for(int shot=0;shot<12;++shot){
            float opening=(shot%3)*.5f,tilt=shot<6?14.f:40.4f;
            auto chair=beachChairTransform(beachChairs[0]);
            glm::vec3 base(beachChairs[0].x-1.6f,0,beachChairs[0].z-1.2f);
            base.y=getCoastalTerrainElevation(base.x,base.z);
            auto umbrella=glm::translate(glm::mat4(1),base);
            glm::vec3 target=base+glm::vec3(-.8f,2,0),eye=target+glm::vec3(4,shot%6<3?3.f:-.6f,4);
            auto view=glm::lookAt(eye,target,glm::vec3(0,1,0));
            auto projection=glm::perspective(glm::radians(45.f),1.5f,.1f,2000.f);
            objects.Activate();glUniform3fv(objects.location("viewPos"),1,glm::value_ptr(eye));
            glUniform3fv(objects.location("lightDir"),1,glm::value_ptr(light));
            glUniform3f(objects.location("lightColor"),1,.98f,.92f);glUniform1f(objects.location("ambientStrength"),.42f);
            auto furniture=[&](){drawIslandTerrainBackdrop(objects,terrain,cube,sphere,&textures);
                drawBeachChair(objects,cube,chair);
                auto seatedLegs=seatedBeachRobotLegs();
                drawRobot(objects,cube,sphere,seatedBeachRobotTransform(chair),15,-5,-20,30,0,.16f,&seatedLegs);
                drawUmbrella(objects,cube,sphere,canopy,umbrella,tilt,3.4f,1,opening);};
            shadows.begin(objects,light,40,target);furniture();shadows.end(objects,960,640,view,projection);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(view,projection,12,light);
            shadows.bind(objects,true);furniture();glFinish();require(glGetError()==GL_NO_ERROR,"Folding umbrella rendering failed");
            std::string path="build/validation/umbrella-fold-"+std::to_string(shot)+".bmp";saveFrame(path.c_str());
        }
        for(int shot=0;shot<5;++shot)
        {
            glm::vec3 target(-6,lighthouseSite.baseHeight()+7,-110);
            float angle=glm::radians(30.0f+shot*90.0f);
            glm::vec3 eye=target+glm::vec3(cosf(angle)*30,5,sinf(angle)*30);
            glm::mat4 view=glm::lookAt(eye,target,glm::vec3(0,1,0));
            glm::mat4 projection=glm::perspective(glm::radians(45.0f),1.5f,.1f,2000.0f);
            objects.Activate();
            glUniform3fv(glGetUniformLocation(objects.ID,"viewPos"),1,glm::value_ptr(eye));
            if(shot==4)
            {
                glUniform3f(glGetUniformLocation(objects.ID,"lightColor"),.18f,.24f,.45f);
                glUniform1f(glGetUniformLocation(objects.ID,"ambientStrength"),.25f);
            }
            auto landmark=[&]() {
                drawIslandTerrainBackdrop(objects,terrain,cube,sphere,&textures);
                lighthouseSite.draw(objects);
                drawLighthouse(objects,cube,sphere,tower,cylinder,roof,lighthouseSite.transform(),0,shot==4?1.f:0.f);
            };
            shadows.begin(objects,light,45,target);landmark();shadows.end(objects,960,640,view,projection);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(view,projection,shot==4?22:12,light);
            shadows.bind(objects,true);landmark();
            if(shot==4)drawLighthouseBeacon(objects,beam,lighthouseSite.transform(),0,1,15,1);
            drawLighthouseGlass(objects,cube,lighthouseSite.transform(),eye,shot==4?1.f:0.f);
            glFinish();require(glGetError()==GL_NO_ERROR,"Lighthouse rendering failed");
            saveFrame(shot==0?"build/validation/lighthouse-east.bmp":shot==1?"build/validation/lighthouse-north.bmp":shot==2?"build/validation/lighthouse-west.bmp":shot==3?"build/validation/lighthouse-south.bmp":"build/validation/lighthouse-night.bmp");
        }
        glm::vec3 forestEye(175,150,110),forestTarget(0,18,-100);
        glm::mat4 forestView=glm::lookAt(forestEye,forestTarget,glm::vec3(0,1,0));
        glm::mat4 forestProjection=glm::perspective(glm::radians(50.0f),1.5f,.1f,2000.0f);
        objects.Activate();
        glUniform3fv(glGetUniformLocation(objects.ID,"viewPos"),1,glm::value_ptr(forestEye));
        glUniform3f(glGetUniformLocation(objects.ID,"lightColor"),1,.98f,.92f);glUniform1f(glGetUniformLocation(objects.ID,"ambientStrength"),.42f);
        glUniformMatrix4fv(glGetUniformLocation(objects.ID,"view"),1,GL_FALSE,glm::value_ptr(forestView));
        glUniformMatrix4fv(glGetUniformLocation(objects.ID,"proj"),1,GL_FALSE,glm::value_ptr(forestProjection));
        shadows.bind(objects,false);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(forestView,forestProjection,12,light);objects.Activate();
        drawIslandTerrainBackdrop(objects,terrain,cube,sphere,&textures);vegetation.draw(objects,7.5f);
        lighthouseSite.draw(objects);drawLighthouse(objects,cube,sphere,tower,cylinder,roof,lighthouseSite.transform(),0);
        saveFrame("build/validation/tropical-growth.bmp");
        GLint panelPreviousProgram;glGetIntegerv(GL_CURRENT_PROGRAM,&panelPreviousProgram);
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glDisable(GL_BLEND);
        dayPanel.draw(960,640,.35f,false);
        GLint panelProgram,polygonMode[2];GLboolean depthMask;
        glGetIntegerv(GL_CURRENT_PROGRAM,&panelProgram);glGetIntegerv(GL_POLYGON_MODE,polygonMode);glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask);
        require(panelProgram==panelPreviousProgram && polygonMode[0]==GL_LINE && glIsEnabled(GL_DEPTH_TEST) && depthMask && !glIsEnabled(GL_BLEND),"Day panel leaks render state");
        glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);glFinish();saveFrame("build/validation/day-speed-panel.bmp");
        require(glGetError()==GL_NO_ERROR,"Day slider rendering failed");
        objects.Activate();glEnable(GL_BLEND);glBlendFuncSeparate(GL_ONE,GL_ZERO,GL_SRC_ALPHA,GL_ONE);
        glDisable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glDepthMask(GL_FALSE);
        glUniform1f(objects.location("objectAlpha"),.37f);
        for(float heading:{0.f,90.f,180.f,270.f})for(float tilt:{-5.f,0.f,5.f})
            drawLighthouseBeacon(objects,beam,lighthouseSite.transform(),0,1,heading,tilt);
        drawLighthouseGlass(objects,cube,lighthouseSite.transform(),forestEye,1);
        GLint src,dst,srcA,dstA;float restoredAlpha;
        glGetIntegerv(GL_BLEND_SRC_RGB,&src);glGetIntegerv(GL_BLEND_DST_RGB,&dst);
        glGetIntegerv(GL_BLEND_SRC_ALPHA,&srcA);glGetIntegerv(GL_BLEND_DST_ALPHA,&dstA);
        glGetUniformfv(objects.ID,objects.location("objectAlpha"),&restoredAlpha);glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask);
        require(glIsEnabled(GL_BLEND)&&!glIsEnabled(GL_DEPTH_TEST)&&glIsEnabled(GL_CULL_FACE)&&!depthMask&&
            src==GL_ONE&&dst==GL_ZERO&&srcA==GL_SRC_ALPHA&&dstA==GL_ONE&&fabsf(restoredAlpha-.37f)<.0001f,"Lantern/beam leaks render state");
        glUniform1f(objects.location("objectAlpha"),1);glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glDepthMask(GL_TRUE);
        // Exercise instanced geometry in each shading mode without changing
        // the original reference renders above.
        for(int mode=0;mode<4;++mode){
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);objects.Activate();
            glUniform1i(objects.location("forceShadingMode"),mode);
            drawIslandTerrainBackdrop(objects,terrain,cube,sphere,&textures,true,false);forest.draw(objects,textures,true);
            glFinish();require(glGetError()==GL_NO_ERROR,"Instanced forest render failed");
        }
        glUniform1i(objects.location("forceShadingMode"),0);
        forest.cleanup();
        for(int width:{480,960,1600}){
            dayPanel.draw(width,width*2/3,.35f,false,{"ATV 12.5 KM/H - TOUR 50% CHASE","WASD DRIVE SPACE BRAKE U TOUR E VIEW H EXIT","ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789 [] () : + - / %","GPU: NVIDIA GEFORCE RTX 3050 LAPTOP GPU"},&guide);
            glGetIntegerv(GL_CURRENT_PROGRAM,&panelProgram);glGetBooleanv(GL_DEPTH_WRITEMASK,&depthMask);
            require(panelProgram==panelPreviousProgram && depthMask && glIsEnabled(GL_DEPTH_TEST) && !glIsEnabled(GL_BLEND),"HUD leaks render state on resize");
            require(glGetError()==GL_NO_ERROR,"HUD resize rendering failed");}
        dayPanel.cleanup();
        lighthouseSite.cleanup();tower.cleanup();roof.cleanup();beam.cleanup();
        g_sceneSeaLevel=0;canopy.cleanup();vegetation.cleanup();shadows.cleanup();tracks.cleanup();terrain.cleanup();ocean.cleanup();cube.cleanup();sphere.cleanup();cylinder.cleanup();
        sky.cleanup();textures.cleanup();objects.Delete();water.Delete();
        require(objects.uniformLocations.empty() && objects.ID==0,"Deleted shader retains cached uniforms");
        glfwDestroyWindow(window);glfwTerminate();
        std::cout<<"PASS: charging eligibility, held-Q detection, charge timing, drive lock/disconnect, depletion and collision-safe parking.\n";
        std::cout<<"PASS: umbrella timing at 10/20/30/60 FPS, held-key/reversal, sampled pole/terrain/occupant clearance and 12 folding/shadow previews.\n";
        std::cout<<"PASS: forest batching/culling/shading, uniform cache lifecycle, house footing support, four-direction swept collisions, reverse-away, obstacle-aware tours at 10/20/30/60 FPS, blocked-tour stop and resized HUD state restoration.\n";
        std::cout<<"PASS: "<<circuit.length<<" m closed beach route, 4096 full-footprint/rider clearances (inset "
            <<smallest<<".."<<largest<<"), 10/20/30/60 FPS driving, tracks, high-tide clearance, tide/shadows/shading, "<<vegetation.palmCount()<<" palms ("<<vegetation.fanPalmCount()<<" fan palms), "<<vegetation.broadleafCount()<<" broadleaf trees, "<<vegetation.understoryCount()<<" tropical plants, "<<vegetation.fernCount()<<" ferns, "<<vegetation.flowerCount()<<" flowering shrubs, "<<vegetation.grassCount()<<" grass clumps, "<<nightLights.count()<<" clear lamp positions, dusk/dawn fade, measured night illumination, lighthouse footprint support, beam controls, slider input/render-state checks and 22 rendered previews.\n";
        return 0;
    }
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
