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

int main()
{
    try
    {
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
        CubeMesh cube;SphereMesh sphere;WaterGridMesh ocean;Skybox sky;
        TextureManager textures;textures.init();
        glEnable(GL_DEPTH_TEST);glViewport(0,0,960,640);
        for(int shot=0;shot<4;++shot)
        {
            float time=7.5f;
            glm::vec3 pos(8,0,78), light=glm::normalize(glm::vec3(-0.4f,0.8f,0.3f));
            BoatFloatState state;
            glm::mat4 boat=floatingBoatTransform(state,pos,20,1.65f,time,0.016f,shot==1?7.0f:0.0f,shot==1?8.0f:0.0f);
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
            }
            std::vector<glm::vec4> trail;
            if(shot==1) for(int i=0;i<32;++i)
            {
                float age=(31-i)*0.25f;
                trail.emplace_back(pos.x-1.2f*age-0.15f*age*age,pos.z-3.5f-6*age,time-age,7.0f);
            }
            drawOceanWaves(water,ocean,view,projection,eye,light,glm::vec3(1,0.98f,0.92f),0.42f,time,
                &textures,true,pos,20,shot==1?7.0f:0,glm::vec4(0),glm::vec4(0),trail);
            glFinish();
            require(glGetError()==GL_NO_ERROR,"OpenGL error during boat/water rendering");
            saveFrame(shot==0?"build/validation/boat-stationary.bmp":shot==1?"build/validation/boat-underway.bmp":shot==2?"build/validation/boat-low-view.bmp":"build/validation/boat-sails.bmp",960,640);
        }
        boatHull(false).cleanup();boatHull(true).cleanup();ocean.cleanup();cube.cleanup();sphere.cleanup();
        boatSail(false).cleanup();boatSail(true).cleanup();
        sky.cleanup();textures.cleanup();objects.Delete();water.Delete();
        glfwDestroyWindow(window);glfwTerminate();
        std::cout<<"PASS: CPU/GPU wave agreement at low/mean/high tide, wave inversion, frame-rate response, 400 camera constraints, constrained transitions, GLSL linking and 4 rendered previews.\n";
        return 0;
    }
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
