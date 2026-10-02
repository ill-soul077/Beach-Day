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
        require(manual.heading>oldHeading,"Right steering turns the wrong way");
        require(fabsf(manual.wheelSpin)>1,"Tyres do not rotate while driving");
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
        FrustumMesh tower(.5f,.34f,40),roof(.55f,0,40),beam(.5f,0,48,false);
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
        IslandTerrainMesh terrain;WaterGridMesh ocean;Skybox sky;
        TextureManager textures;textures.init();
        IslandNightLights nightLights(circuit);
        IslandVegetation vegetation(circuit);
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
                drawLighthouse(objects,cube,sphere,tower,cylinder,roof,lighthouseSite.transform(),0);
            };
            shadows.begin(objects,light,45,target);landmark();shadows.end(objects,960,640,view,projection);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(view,projection,shot==4?22:12,light);
            shadows.bind(objects,true);landmark();
            if(shot==4)drawLighthouseBeacon(objects,beam,lighthouseSite.transform(),0,1,15,1);
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
        require(glGetError()==GL_NO_ERROR,"Day slider rendering failed");dayPanel.cleanup();
        lighthouseSite.cleanup();tower.cleanup();roof.cleanup();beam.cleanup();
        g_sceneSeaLevel=0;canopy.cleanup();vegetation.cleanup();shadows.cleanup();tracks.cleanup();terrain.cleanup();ocean.cleanup();cube.cleanup();sphere.cleanup();cylinder.cleanup();
        sky.cleanup();textures.cleanup();objects.Delete();water.Delete();
        glfwDestroyWindow(window);glfwTerminate();
        std::cout<<"PASS: "<<circuit.length<<" m closed beach route, 4096 full-footprint/rider clearances (inset "
            <<smallest<<".."<<largest<<"), 10/20/30/60 FPS driving, tracks, high-tide clearance, tide/shadows/shading, "<<vegetation.palmCount()<<" palms ("<<vegetation.fanPalmCount()<<" fan palms), "<<vegetation.broadleafCount()<<" broadleaf trees, "<<vegetation.understoryCount()<<" tropical plants, "<<vegetation.fernCount()<<" ferns, "<<vegetation.flowerCount()<<" flowering shrubs, "<<vegetation.grassCount()<<" grass clumps, "<<nightLights.count()<<" clear lamp positions, dusk/dawn fade, measured night illumination, lighthouse footprint support, beam controls, slider input/render-state checks and 22 rendered previews.\n";
        return 0;
    }
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
