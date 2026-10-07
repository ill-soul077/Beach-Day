#include "Scene.h"
#include <fstream>
#include <stdexcept>
#include <cstdint>

static void require(bool condition,const char* message)
{
    if(!condition) throw std::runtime_error(message);
}

static void checkProgram(Shader& shader)
{
    GLint linked=0; glGetProgramiv(shader.ID,GL_LINK_STATUS,&linked);
    require(linked==GL_TRUE,"GLSL program did not link");
}

static void saveFrame(const char* path,int width,int height)
{
    std::vector<unsigned char> pixels(width*height*3);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
    for(size_t i=0;i<pixels.size();i+=3) std::swap(pixels[i],pixels[i+2]);
    std::ofstream out(path,std::ios::binary);
    auto word=[&](uint16_t value){out.write((char*)&value,2);};
    auto dword=[&](uint32_t value){out.write((char*)&value,4);};
    word(0x4d42);dword(54+(uint32_t)pixels.size());dword(0);dword(54);
    dword(40);dword(width);dword(height);word(1);word(24);
    dword(0);dword((uint32_t)pixels.size());dword(2835);dword(2835);dword(0);dword(0);
    out.write((char*)pixels.data(),pixels.size());
    require(out.good(),"Could not save render preview");
}

static void verifyGPUWaveHeights(Shader& water)
{
    const char* varyings[]={"FragPos"};
    glTransformFeedbackVaryings(water.ID,1,varyings,GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(water.ID);checkProgram(water);water.Activate();
    glm::mat4 identity(1);
    for(const char* name:{"model","view","proj"})
        glUniformMatrix4fv(glGetUniformLocation(water.ID,name),1,GL_FALSE,glm::value_ptr(identity));
    glUniform1f(glGetUniformLocation(water.ID,"time"),7.5f);
    glUniform1f(glGetUniformLocation(water.ID,"boatSpeed"),0);
    std::vector<glm::vec3> points;
    for(float z:{55.0f,78.0f,108.0f,180.0f})
        for(float x:{-60.0f,-8.0f,8.0f,45.0f}) points.emplace_back(x,0,z);
    std::vector<glm::vec3> output(points.size());
    GLuint vao,input,feedback;glGenVertexArrays(1,&vao);glGenBuffers(1,&input);glGenBuffers(1,&feedback);
    glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,input);
    glBufferData(GL_ARRAY_BUFFER,points.size()*sizeof(glm::vec3),points.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(glm::vec3),nullptr);glEnableVertexAttribArray(0);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,feedback);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,output.size()*sizeof(glm::vec3),nullptr,GL_STREAM_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,feedback);
    for(float level:{-.22f,0.0f,.22f})
    {
    g_sceneSeaLevel=level;
    glUniform1f(glGetUniformLocation(water.ID,"seaLevel"),level);
    glEnable(GL_RASTERIZER_DISCARD);glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS,0,(GLsizei)points.size());glEndTransformFeedback();glDisable(GL_RASTERIZER_DISCARD);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER,0,output.size()*sizeof(glm::vec3),output.data());
    for(size_t i=0;i<points.size();++i)
    {
        glm::vec3 cpu,normal;getGerstnerWaveAt(points[i].x,points[i].z,7.5f,cpu,normal);
        require(glm::distance(cpu,output[i])<0.002f,"Buoyancy waves differ from rendered GPU waves");
    }
    }
    g_sceneSeaLevel=0;glUniform1f(glGetUniformLocation(water.ID,"seaLevel"),0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,0);glBindVertexArray(0);
    glDeleteBuffers(1,&input);glDeleteBuffers(1,&feedback);glDeleteVertexArrays(1,&vao);
}

static void verifyHullMaskAndWake()
{
    // Probe the production fragment shader directly. This catches holes in
    // the masking math independently of the camera or ocean triangulation.
    const char* vertex=R"GLSL(#version 330 core
uniform vec3 probe;
out vec3 FragPos;out vec3 Normal;out vec2 TexCoord;
out float WaveElevation;out float WaterDepth;out float ShoreDistance;
out float WakeFactor;out float BowWaveFactor;out vec2 WaterLighting;
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);
gl_Position=vec4(p*2-1,0,1);FragPos=probe;Normal=vec3(0,1,0);TexCoord=vec2(0);
WaveElevation=0;WaterDepth=4;ShoreDistance=50;WakeFactor=0;BowWaveFactor=0;WaterLighting=vec2(.8,0);}
)GLSL";
    auto compile=[](GLenum type,const char* code){
        GLuint shader=glCreateShader(type);glShaderSource(shader,1,&code,nullptr);glCompileShader(shader);
        GLint ok;glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);require(ok==GL_TRUE,"Probe shader failed to compile");return shader;
    };
    GLuint vs=compile(GL_VERTEX_SHADER,vertex),fs=compile(GL_FRAGMENT_SHADER,BeachDayShaders::WaterFragment);
    GLuint program=glCreateProgram();glAttachShader(program,vs);glAttachShader(program,fs);glLinkProgram(program);
    GLint ok;glGetProgramiv(program,GL_LINK_STATUS,&ok);require(ok==GL_TRUE,"Water probe failed to link");
    glDeleteShader(vs);glDeleteShader(fs);glUseProgram(program);
    auto location=[&](const char* name){return glGetUniformLocation(program,name);};
    glUniform3f(location("viewPos"),8,7,12);glUniform3f(location("lightDir"),0,1,0);
    glUniform3f(location("lightColor"),1,1,1);glUniform1f(location("ambientStrength"),.42f);
    glUniform1f(location("time"),7.5f);glUniform1i(location("hullMaskEnabled"),1);
    GLuint vao,query;glGenVertexArrays(1,&vao);glBindVertexArray(vao);glGenQueries(1,&query);
    glViewport(0,0,1,1);glDisable(GL_DEPTH_TEST);glDisable(GL_BLEND);
    auto survives=[&](glm::vec3 point){
        glUniform3fv(location("probe"),1,glm::value_ptr(point));
        glBeginQuery(GL_ANY_SAMPLES_PASSED,query);glDrawArrays(GL_TRIANGLES,0,3);glEndQuery(GL_ANY_SAMPLES_PASSED);
        GLuint passed;glGetQueryObjectuiv(query,GL_QUERY_RESULT,&passed);return passed!=0;
    };
    for(int hull=0;hull<3;++hull)for(float tilt:{-12.f,0.f,12.f})
    {
        glm::vec4 hulls[3]={};hulls[hull]={8,78,37,1.65f};
        glm::mat4 pose=glm::translate(glm::mat4(1),glm::vec3(8,.22f,78));
        pose=glm::rotate(pose,glm::radians(37.f),glm::vec3(0,1,0));
        pose=glm::rotate(pose,glm::radians(tilt),glm::vec3(1,0,0));
        pose=glm::rotate(pose,glm::radians(-tilt*.6f),glm::vec3(0,0,1));
        pose=glm::scale(pose,glm::vec3(1.65f));
        glm::mat4 inverses[3]={glm::mat4(1),glm::mat4(1),glm::mat4(1)};inverses[hull]=glm::inverse(pose);
        glUniform4fv(location("hulls[0]"),3,glm::value_ptr(hulls[0]));
        glUniformMatrix4fv(location("inverseHulls[0]"),3,GL_FALSE,glm::value_ptr(inverses[0]));
        for(glm::vec3 p:std::vector<glm::vec3>{{.1f,.18f,-.4f},{.1f,.28f,.7f},{0,.05f,-1.3f}})
            require(!survives(glm::vec3(pose*glm::vec4(p,1))),"Ocean fragment remains inside pitched/rolled hull");
        for(glm::vec3 p:std::vector<glm::vec3>{{.9f,.10f,-.4f},{0,-.5f,-.4f},{0,.7f,-.4f},{0,.1f,-2.1f},{0,.4f,2.4f}})
            require(survives(glm::vec3(pose*glm::vec4(p,1))),"Hull mask removes exterior ocean water");
    }
    glUniform1i(location("hullMaskEnabled"),0);
    glm::vec4 hulls[3]={{8,78,0,1.65f},{0,0,0,0},{0,0,0,0}};
    glUniform4fv(location("hulls[0]"),3,glm::value_ptr(hulls[0]));
    // Both directions must brighten the trailing water; stationary boats
    // must not have a moving propeller-wash stripe.
    for(float speed:{7.f,-7.f})
    {
        glm::vec3 p(8,0,speed>0?70:86);
        glUniform3fv(location("probe"),1,glm::value_ptr(p));
        float pixels[3],still[3];glUniform1f(location("boatSpeed"),0);glDrawArrays(GL_TRIANGLES,0,3);
        glReadPixels(0,0,1,1,GL_RGB,GL_FLOAT,still);
        glUniform1f(location("boatSpeed"),speed);glDrawArrays(GL_TRIANGLES,0,3);
        glReadPixels(0,0,1,1,GL_RGB,GL_FLOAT,pixels);
        require(pixels[0]+pixels[1]+pixels[2]>still[0]+still[1]+still[2]+.05f,"Moving boat wake is not visible in forward/reverse");
    }
    glDeleteQueries(1,&query);glDeleteVertexArrays(1,&vao);glDeleteProgram(program);
    glEnable(GL_DEPTH_TEST);glViewport(0,0,960,640);
}

static glm::mat4 robotFoot(const glm::mat4& root,const RobotLegPose& legs,int side)
{
    int i=side==-1?0:1;
    glm::mat4 m=glm::translate(root,{side*.16f,-legs.hipDrop,.04f});
    m=glm::rotate(m,glm::radians(legs.hipPitch[i]),glm::vec3(1,0,0));
    m=glm::translate(m,{0,-.39f,0});
    m=glm::rotate(m,glm::radians(legs.kneePitch[i]),glm::vec3(1,0,0));
    m=glm::translate(m,{0,-legs.shinLength,0});
    m=glm::rotate(m,glm::radians(legs.anklePitch[i]),glm::vec3(1,0,0));
    return glm::translate(m,{0,0,.09f});
}
static void verifyBeachRobots()
{
    auto seat=seatedBeachRobotTransform(glm::mat4(1));auto legs=seatedBeachRobotLegs();
    require(fabsf(seat[3].y-.10f-.4975f-.005f)<.0001f,"Seated pelvis is not supported by chair");
    for(int side:{-1,1})
    {
        auto foot=robotFoot(seat,legs,side);
        require(foot[3].z-.14f>.725f,"Seated foot or shin intersects chair front");
        require(fabsf(foot[3].y-.05f)<.015f,"Seated feet do not reach sand");
        require(seat[3].y-legs.hipDrop-.07f>=.4974f,"Seated thigh penetrates seat");
    }
    BeachRobotState keys;
    require(keys.press(true)&&keys.active,"Enter cannot enable walking");
    require(!keys.press(true)&&keys.active,"Held Enter repeats walking toggle");
    keys.press(false);require(keys.press(true)&&!keys.active,"Enter cannot leave walking");
    {
        BeachRobotState turning;turning.active=true;auto start=turning.pos;float yaw=turning.heading;
        updateBeachRobot(turning,0,1,.25f,{});
        float pivotAngle=fabsf(turning.heading-yaw);
        require(glm::distance(start,turning.pos)<.001f&&pivotAngle>15&&pivotAngle<25&&turning.gaitBlend>.5f,
            "Turning in place lacks stepping animation or changes position");
    }
    BeachCircuit circuit;float heading;auto start=circuit.sample(0,&heading);
    glm::vec3 reference(0);
    for(int fps:{10,20,30,60})
    {
        BeachRobotState state;state.pos=start;state.heading=heading;state.active=true;
        std::vector<SolidObstacle> obstacles={SolidObstacle::circle(start,.46f)};
        for(int frame=0;frame<fps*4;++frame)updateBeachRobot(state,1,0,1.f/fps,obstacles);
        if(fps==10)reference=state.pos;
        else require(glm::distance(reference,state.pos)<.04f,"Walking speed depends on frame rate");
        updateBeachRobot(state,0,0,.25f,obstacles);updateBeachRobot(state,0,0,.25f,obstacles);
        require(fabsf(state.speed)<.001f,"Walking does not stop when keys released");
        state.active=false;auto before=state.pos;updateBeachRobot(state,1,1,.2f,obstacles);
        require(glm::distance(before,state.pos)<.001f,"Inactive walking controls move robot");
        state.active=true;
        glm::vec3 forward(sinf(glm::radians(heading)),0,cosf(glm::radians(heading)));
        state.pos=start;state.heading=heading;state.speed=0;
        obstacles.push_back(SolidObstacle::box(glm::rotate(glm::translate(glm::mat4(1),start+forward*2.f),glm::radians(heading),glm::vec3(0,1,0)),{.5f,.4f}));
        for(int i=0;i<fps*3;++i)updateBeachRobot(state,1,0,1.f/fps,obstacles);
        require(state.blocked&&glm::distance(state.pos,start)<1.2f,"Walking tunnels through solid obstacle");
        auto stopped=state.pos;
        for(int i=0;i<fps;++i)updateBeachRobot(state,-1,0,1.f/fps,obstacles);
        require(glm::distance(state.pos,stopped)>.5f&&!state.blocked,"Robot cannot reverse away from contact");
    }
    {
        BeachRobotState state;state.pos=start;state.active=true;
        glm::vec2 inward(
            getIslandSignedDistance(start.x+.1f,start.z)-getIslandSignedDistance(start.x-.1f,start.z),
            getIslandSignedDistance(start.x,start.z+.1f)-getIslandSignedDistance(start.x,start.z-.1f));
        inward=glm::normalize(inward);
        state.heading=glm::degrees(atan2f(-inward.x,-inward.y));
        for(int i=0;i<240;++i)updateBeachRobot(state,1,0,1/30.f,{});
        require(state.blocked&&getIslandSignedDistance(state.pos.x,state.pos.z)>=5,"Walking enters ocean beyond dry path");
        state.pos=start;state.heading=heading;state.speed=0;
        BeachATVState parked;parked.heading=heading;
        parked.pos=start+glm::vec3(sinf(glm::radians(heading)),0,cosf(glm::radians(heading)))*3.f;
        for(int i=0;i<120;++i)updateBeachRobot(state,1,0,1/30.f,{},&parked);
        require(state.blocked&&!atvTouchesObstacle(parked.pos,parked.heading,SolidObstacle::circle(state.pos,.46f)),
            "Walking robot passes through parked ATV");
    }
    for(float distance=0;distance<circuit.length;distance+=8)
    for(int step=0;step<16;++step)
    {
        BeachRobotState state;state.pos=circuit.sample(distance,&state.heading);
        state.phase=step*6.283185307f/16;state.gaitBlend=1;
        auto pose=beachRobotPose(state);
        for(int side:{-1,1})
        {
            auto foot=robotFoot(pose.transform,pose.legs,side);
            float ground=getCoastalTerrainElevation(foot[3].x,foot[3].z);
            float clearance=foot[3].y-.05f-ground;
            require(clearance>=-.005f&&clearance<.14f,"Walking foot sinks or floats beyond gait lift");
            require(fabsf(foot[1].y-1)<.001f,"Walking ankle does not keep foot level");
            require(getIslandSignedDistance(foot[3].x,foot[3].z)>4.5f,"Walking foot enters sea");
        }
    }
    std::cout<<"PASS: seated seat/thigh/frame/foot clearances; held Enter, walking at 10/20/30/60 FPS, release/exit, solid contact/reverse and terrain-foot gait around full perimeter.\n";
}

static void verifyBoatContactsAndSea(){
    std::vector<SolidObstacle> none;
    auto obstacle=SolidObstacle::box(glm::rotate(glm::translate(glm::mat4(1),glm::vec3(500,0,500)),glm::radians(35.f),glm::vec3(0,1,0)),{1.f,3.f});
    std::vector<SolidObstacle> boats{obstacle};glm::vec3 reference(0);
    for(int fps:{10,20,30,60}){
        BoatContactPose pose{{500,0,480},0};bool hit=false;
        for(int frame=0;frame<fps*4;++frame){pose=sweepBoatMotion(pose.pos,pose.heading,pose.pos+glm::vec3(0,0,12.f/fps),pose.heading,none,boats);if(pose.blocked){hit=true;break;}}
        require(hit&&boatPoseClear(pose.pos,pose.heading,none,boats),"Boat collision tunnels through another hull");
        if(fps==10)reference=pose.pos;else require(glm::distance(reference,pose.pos)<.002f,"Boat contact depends on frame rate");
        auto reverse=sweepBoatMotion(pose.pos,pose.heading,pose.pos-glm::vec3(0,0,1),pose.heading,none,boats);
        require(!reverse.blocked,"Boat cannot reverse away from contact");
    }
    for(float heading:{0.f,90.f,180.f,270.f}){
        float a=glm::radians(heading);glm::vec3 f(sinf(a),0,cosf(a)),centre(500,0,500);
        std::vector<SolidObstacle> pole{SolidObstacle::circle(centre,.12f)};
        auto contact=sweepBoatMotion(centre-f*15.f,heading,centre+f*15.f,heading,pole,none);
        require(contact.blocked&&boatPoseClear(contact.pos,contact.heading,pole,none),"Boat swept footprint misses a thin post");
    }
    auto coast=sweepBoatMotion({0,0,110},180,{0,0,-110},180,none,none);
    require(coast.blocked&&boatPoseClear(coast.pos,coast.heading,none,none),"Boat runs through the island");
    require(!boatPoseClear({WaterGridMesh::boatLimit+1,0,500},0,none,none),"Boat leaves navigable ocean bounds");
    require(WaterGridMesh::halfExtent>WaterGridMesh::cameraLimit+600+WaterGridMesh::viewDistance,"Ocean edge can enter an allowed camera frustum");
    float previous=-1e30f;
    for(int i=0;i<=300;++i){float t=i/300.f,u=2*t-1,old=copysignf(2600*powf(fabsf(u),1.6f),u);
        float ocean=WaterGridMesh::coordinate(t,-WaterGridMesh::halfExtent,WaterGridMesh::halfExtent);
        require(ocean>=previous,"Extended ocean grid folds back on itself");previous=ocean;
        if(fabsf(old)<=700)require(fabsf(ocean-old)<.001f,"Ocean expansion changes near-island tessellation");
    }
    std::cout<<"PASS: swept boat/boat, shoreline and thin-post contacts, reverse-away at 10/20/30/60 FPS; ocean extent, preserved nearshore grid and camera-frustum edge clearance.\n";
}
int main()
{
    try
    {
        verifyBeachRobots();
        verifyBoatContactsAndSea();
        // Exercise inversion over open ocean and the near-shore shelf.
        for(float z=48;z<190;z+=7) for(float x=-25;x<26;x+=5)
        {
            glm::vec3 p,n;getWaterSurfaceAt(x,z,7.5f,p,n);
            require(glm::length(glm::vec2(p.x-x,p.z-z))<0.003f,"Wave inversion mismatch");
        }
        BoatFloatState fast,slow;
        for(int i=0;i<600;++i)
            floatingBoatTransform(fast,glm::vec3(8,0,78),67,1.65f,i/60.0f,1/60.0f);
        for(int i=0;i<300;++i)
            floatingBoatTransform(slow,glm::vec3(8,0,78),67,1.65f,i/30.0f,1/30.0f);
        require(fabsf(fast.height-slow.height)<0.06f,"Heave response depends excessively on frame rate");
        require(fabsf(fast.pitch-slow.pitch)<0.5f,"Pitch response depends excessively on frame rate");
        require(fabsf(fast.roll-slow.roll)<0.5f,"Roll response depends excessively on frame rate");
        for(int fps:{10,20,30,60})
        {
            BoatFloatState sample;
            for(int i=0;i<fps*10;++i)
                floatingBoatTransform(sample,{8,0,78},67,1.65f,i/(float)fps,1.0f/fps);
            require(std::isfinite(sample.height) && fabsf(sample.height-fast.height)<.12f,"21-point buoyancy unstable across frame rates");
            require(fabsf(sample.pitch-fast.pitch)<1.2f && fabsf(sample.roll-fast.roll)<1.2f,"21-point attitude unstable across frame rates");
        }
        auto standing=standingBeachRobotTransform();
        for(float side:{-1.f,1.f})for(float x:{-.085f,.085f})for(float z:{-.14f,.14f})
        {
            glm::vec3 sole(standing*glm::vec4(side*.16f+x,-.90f,.13f+z,1));
            require(getIslandSignedDistance(sole.x,sole.z)>13,"Standing robot is not on dry inland beach");
            require(fabsf(sole.y-getCoastalTerrainElevation(sole.x,sole.z))<.025f,"Standing robot feet lack terrain support");
        }

        Camera camera;
        camera.realisticNavigation=true;
        camera.surfaceHeight=[](float x,float z){return glm::max(1.15f,getIslandBackdropElevation(x,z));};
        for(int i=0;i<400;++i)
        {
            camera.target=glm::vec3((i*73%300)-150,-20,(i*97%400)-280);
            camera.yaw=(float)(i*37%360);camera.pitch=-10;camera.distance=(float)(i%200+1);
            camera.projMode=PROJ_ORTHO_SIDE;camera.update(0);
            glm::vec3 eye=camera.getPosition();
            require(camera.projMode==PROJ_PERSPECTIVE,"Island camera allowed orthographic cutaway");
            require(eye.y>=camera.surfaceHeight(eye.x,eye.z)+1.64f,"Camera entered water or terrain");
        }
        camera.target=glm::vec3(8,1,78);camera.desiredTarget=glm::vec3(-6,-5,-110);
        camera.desiredDistance=4;camera.desiredYaw=30;camera.desiredPitch=-8;
        camera.isTransitioning=true;
        for(int i=0;i<600;++i) camera.update(1/60.0f);
        require(!camera.isTransitioning,"Camera cannot finish a constrained hill preset transition");
        require(glfwInit()==GLFW_TRUE,"GLFW initialization failed");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
        glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
        glfwWindowHint(GLFW_SAMPLES,4);
        GLFWwindow* window=glfwCreateWindow(960,640,"Boat rendering verification",nullptr,nullptr);
        require(window!=nullptr,"OpenGL test context failed");
        glfwMakeContextCurrent(window);
        require(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)!=0,"GLAD loading failed");
        Shader objects("default.vert","default.frag"),water("water.vert","water.frag");
        checkProgram(objects);checkProgram(water);
        verifyGPUWaveHeights(water);
        verifyHullMaskAndWake();
        CubeMesh cube;SphereMesh sphere;WaterGridMesh ocean;Skybox sky;
        TextureManager textures;textures.init();
        glEnable(GL_DEPTH_TEST);glViewport(0,0,960,640);
        for(int shot=0;shot<4;++shot)
        {
            float time=7.5f;
            glm::vec3 pos(8,0,78), light=glm::normalize(glm::vec3(-0.4f,0.8f,0.3f));
            BoatFloatState state;
            glm::mat4 boat=floatingBoatTransform(state,pos,20,1.65f,time,0.016f,shot==1?7.0f:0.0f,shot==1?8.0f:0.0f);
            std::vector<glm::mat4> hullPoses={boat};
            glm::vec4 sailHull(0);
            glm::vec3 eye=pos+glm::vec3(shot==2?9:10,shot==2?2.9f:5.5f,shot==2?2:12);
            glm::mat4 view=glm::lookAt(eye,pos+glm::vec3(0,0.8f,0),glm::vec3(0,1,0));
            glm::mat4 projection=glm::perspective(glm::radians(40.0f),1.5f,0.1f,2000.0f);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);sky.draw(view,projection,12.0f,light);
            objects.Activate();
            glUniformMatrix4fv(glGetUniformLocation(objects.ID,"view"),1,GL_FALSE,glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(objects.ID,"proj"),1,GL_FALSE,glm::value_ptr(projection));
            glUniform3fv(glGetUniformLocation(objects.ID,"viewPos"),1,glm::value_ptr(eye));
            glUniform3fv(glGetUniformLocation(objects.ID,"lightDir"),1,glm::value_ptr(light));
            glUniform3f(glGetUniformLocation(objects.ID,"lightColor"),1,0.98f,0.92f);
            glUniform1f(glGetUniformLocation(objects.ID,"ambientStrength"),0.42f);
            glUniform1f(glGetUniformLocation(objects.ID,"objectAlpha"),1);
            drawRealisticMotorboat(objects,cube,sphere,boat,shot==1?15.0f:0,shot==1?1.0f:0,45,shot==1?7.0f:0,true);
            if(shot==3)
            {
                BoatFloatState sail;
                glm::mat4 sailTransform=floatingBoatTransform(sail,pos+glm::vec3(-4,0,2),35,1.30f,time,0.016f);
                drawBoat(objects,cube,sphere,sailTransform,&textures);
                hullPoses.push_back(sailTransform);
                sailHull={pos.x-4,pos.z+2,35,1.30f};
            }
            std::vector<glm::vec4> trail;
            if(shot==1) for(int i=0;i<32;++i)
            {
                float age=(31-i)*0.25f;
                trail.emplace_back(pos.x-1.2f*age-0.15f*age*age,pos.z-3.5f-6*age,time-age,7.0f);
            }
            drawOceanWaves(water,ocean,view,projection,eye,light,glm::vec3(1,0.98f,0.92f),0.42f,time,
                &textures,true,pos,20,shot==1?7.0f:0,sailHull,glm::vec4(0),trail,hullPoses);
            glFinish();
            require(glGetError()==GL_NO_ERROR,"OpenGL error during boat/water rendering");
            saveFrame(shot==0?"build/validation/boat-stationary.bmp":shot==1?"build/validation/boat-underway.bmp":shot==2?"build/validation/boat-low-view.bmp":"build/validation/boat-sails.bmp",960,640);
        }
        boatHull(false).cleanup();boatHull(true).cleanup();ocean.cleanup();cube.cleanup();sphere.cleanup();
        boatSail(false).cleanup();boatSail(true).cleanup();
        sky.cleanup();textures.cleanup();objects.Delete();water.Delete();
        glfwDestroyWindow(window);glfwTerminate();
        std::cout<<"PASS: CPU/GPU waves and tide, 21-point buoyancy at 10/20/30/60 FPS, dry supported standing robot, pitched/rolled hull masks and exterior clearance, forward/reverse wake pixels, 400 camera constraints, GLSL linking and 4 previews.\n";
        return 0;
    }
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
