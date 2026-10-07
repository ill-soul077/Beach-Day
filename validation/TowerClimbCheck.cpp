// Real render-loop captures and automated controls/route checks; hidden window.
#include "../Scene.h"
#include <fstream>
#include <stdexcept>
static int testFrame=0;
static std::string testView,testOutput;
static double testCursorX=0,testCursorY=0;
static void testCursor(GLFWwindow* window,double* x,double* y){if(testView=="console"){*x=testCursorX;*y=testCursorY;}else glfwGetCursorPos(window,x,y);}
extern TowerClimbState g_towerRobot;
extern int g_towerViewMode;
extern bool g_towerFirstPerson;
extern bool g_towerManual;
extern bool g_dayPaused;
extern BeaconControlPanel g_beaconPanel;
extern LighthouseBeamState g_lighthouseBeam;
extern Camera camera;
extern float g_lighthouseBaseHeight;
extern float g_towerLookYaw,g_towerLookPitch;
void mouse_button_callback(GLFWwindow*,int,int,int);
void cursor_position_callback(GLFWwindow*,double,double);
static void prepareTowerFrame();
static void checkSteeringFrame();
static void requireTower(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static double testTime(){prepareTowerFrame();return 12.0+testFrame/60.0;}
static int testKey(GLFWwindow*,int key){
    if(testView=="boat-right"||testView=="boat-left"){
        if(testFrame==0&&key==GLFW_KEY_F)return GLFW_PRESS;
        if(testFrame>=3&&testFrame<=10&&(key==GLFW_KEY_W||
            key==(testView=="boat-right"?GLFW_KEY_D:GLFW_KEY_A)))return GLFW_PRESS;
        return GLFW_RELEASE;
    }
    if(testView=="head-controls"){
        if(testFrame>=1&&testFrame<=5&&key==GLFW_KEY_2)return GLFW_PRESS;
        if(testFrame>=7&&testFrame<=11&&key==GLFW_KEY_1)return GLFW_PRESS;
        return GLFW_RELEASE;
    }
    if(testView=="stairs-right"&&testFrame>=3&&testFrame<=5&&key==GLFW_KEY_RIGHT)return GLFW_PRESS;
    if(testFrame==0&&key==((testView=="top-view"||testView=="v-first"||testView=="console")?GLFW_KEY_V:GLFW_KEY_PAGE_UP))return GLFW_PRESS;
    if(testView=="console"&&key==GLFW_KEY_D&&testFrame>=3&&testFrame<=5)return GLFW_PRESS;
    if(testView=="v-first"){
        if(key==GLFW_KEY_E&&(testFrame==2||testFrame==4))return GLFW_PRESS;
        if(key==GLFW_KEY_S&&testFrame>=6&&testFrame<=8)return GLFW_PRESS;
        if(key==GLFW_KEY_W&&((testFrame>=11&&testFrame<=13)||testFrame==23))return GLFW_PRESS;
        if(key==GLFW_KEY_SPACE&&(testFrame==14||testFrame==16||testFrame==18))return GLFW_PRESS;
        if(key==GLFW_KEY_PAGE_DOWN&&testFrame==20)return GLFW_PRESS;
        if(key==GLFW_KEY_ENTER&&testFrame==22)return GLFW_PRESS;
    }
    if(testView=="controls"){
        if(key==GLFW_KEY_SPACE&&(testFrame==2||testFrame==5))return GLFW_PRESS;
        if(key==GLFW_KEY_E&&(testFrame==7||testFrame==9))return GLFW_PRESS;
        if(key==GLFW_KEY_PAGE_DOWN&&testFrame==10)return GLFW_PRESS;
        if(key==GLFW_KEY_V&&testFrame==12)return GLFW_PRESS;
        if(key==GLFW_KEY_T&&testFrame==15)return GLFW_PRESS;
        if(key==GLFW_KEY_PAGE_UP&&(testFrame==18||testFrame==19||testFrame==21))return GLFW_PRESS;
    }
    if(testView=="cutaway"&&key==GLFW_KEY_E&&testFrame==1)return GLFW_PRESS;
    if(testView=="top-switch"&&key==GLFW_KEY_V&&testFrame==5)return GLFW_PRESS;
    return GLFW_RELEASE;
}
static GLFWwindow* testWindow(int w,int h,const char* t,GLFWmonitor* m,GLFWwindow* shared){
    glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);return glfwCreateWindow(w,h,t,m,shared);
}
static void testSwap(GLFWwindow* window){
    static float pausedProgress=0,mouseYaw=0,mousePitch=0,restartedProgress=0;
    checkSteeringFrame();
    if(testFrame==0&&testView!="boat-right"&&testView!="boat-left"&&testView!="head-controls")
        requireTower(g_towerViewMode==((testView=="top-view"||testView=="v-first"||testView=="console")?2:1),"Tower start/view key failed");
    if(testView=="stairs-right"&&testFrame==6)requireTower(g_towerLookYaw<0,"Keeper right arrow looks left");
    if(testView=="console"){
        requireTower(g_beaconPanel.visible,"Gallery console missing");
        auto click=[&](float x,float y){
            testCursorX=g_beaconPanel.left+x*g_beaconPanel.scale;testCursorY=g_beaconPanel.top+y*g_beaconPanel.scale;
            mouse_button_callback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
            mouse_button_callback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
            requireTower(!camera.isDraggingOrbit,"Console click leaked to camera");
        };
        if(testFrame==1){click(170,48);requireTower(g_beaconPanel.power==1,"Beacon power ON failed");}
        if(testFrame==2){click(170,130);requireTower(g_lighthouseBeam.paused&&fabsf(g_lighthouseBeam.heading-179.5f)<.1f,"Manual yaw slider failed");}
        if(testFrame==6)requireTower(g_towerRobot.galleryAngle<-.01f,"D does not move keeper right around gallery");
        if(testFrame==7){click(322,165);requireTower(g_lighthouseBeam.elevation==5,"Tilt slider failed");}
        if(testFrame==8){click(18,202);requireTower(g_lighthouseBeam.speed==5,"Speed slider failed");}
        if(testFrame==9){click(240,86);requireTower(!g_lighthouseBeam.paused,"Auto sweep button failed");}
        if(testFrame==10){click(280,48);requireTower(g_beaconPanel.power==2,"Power OFF failed");}
        if(testFrame==11){click(60,48);requireTower(g_beaconPanel.power==0,"Day/night AUTO power failed");}
        if(testFrame==12){click(170,48);click(18,130);click(170,165);}
    }
    if(testView=="v-first"){
        static float gallery=0,stopped=0;
        if(testFrame==0){gallery=g_towerRobot.progress;requireTower(g_towerRobot.finished(),"V does not place keeper on gallery");}
        if(testFrame==2)requireTower(g_towerViewMode==1&&!g_towerFirstPerson,"E ignored after V");
        if(testFrame==4)requireTower(g_towerFirstPerson,"E does not restore POV after V");
        if(testFrame==8){stopped=g_towerRobot.progress;requireTower(g_towerManual&&stopped<gallery,"S does not descend after V");}
        if(testFrame==10)requireTower(g_towerRobot.progress==stopped,"Manual movement continues after release");
        if(testFrame==13)requireTower(g_towerRobot.progress>stopped,"W does not climb");
        if(testFrame==14)requireTower(!g_towerManual&&g_towerRobot.running&&g_dayPaused,"Space failed to start auto climb or changed day clock");
        if(testFrame==16)requireTower(g_towerRobot.paused,"Space failed to pause after V");
        if(testFrame==18)requireTower(!g_towerRobot.paused&&g_towerRobot.running,"Space failed to resume after V");
        if(testFrame==20)requireTower(g_towerViewMode==0&&g_towerRobot.progress==0,"PageDown failed after V");
        if(testFrame==22)requireTower(g_towerViewMode==1&&g_towerManual,"Enter failed to control tower keeper");
        if(testFrame==23)requireTower(g_towerRobot.progress>0,"W failed from entrance");
    }
    if(testView=="controls"){
        if(testFrame==2)pausedProgress=g_towerRobot.progress;
        if(testFrame==3)requireTower(g_towerRobot.paused&&g_towerRobot.progress==pausedProgress,"Space did not stop climb progress");
        if(testFrame==6)requireTower(!g_towerRobot.paused,"Space did not resume climb");
        if(testFrame==8)requireTower(!g_towerFirstPerson,"E did not switch to cutaway");
        if(testFrame==9)requireTower(g_towerFirstPerson,"E did not restore robot POV");
        if(testFrame==11)requireTower(g_towerViewMode==0&&g_towerRobot.progress==0,"PageDown did not reset and exit");
        if(testFrame==13)requireTower(g_towerViewMode==2,"V did not open gallery view");
        if(testFrame==13){
            // Use the production GLFW callbacks, away from the HUD panels.
            mouseYaw=g_towerLookYaw;mousePitch=g_towerLookPitch;
            glfwSetCursorPos(window,850,450);
            mouse_button_callback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
            cursor_position_callback(window,950,410);
            mouse_button_callback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
            requireTower(fabsf(g_towerLookYaw-mouseYaw)>10&&fabsf(g_towerLookPitch-mousePitch)>3,
                "Left drag did not update tower look angles");
        }
        if(testFrame==16)requireTower(g_towerViewMode==0,"T did not restore exterior camera");
        if(testFrame==18){
            requireTower(g_towerViewMode==1&&g_towerFirstPerson&&g_towerRobot.progress<.1f,
                "PageUp did not start a fresh climb");
            restartedProgress=g_towerRobot.progress;
        }
        if(testFrame==19)requireTower(g_towerRobot.progress>restartedProgress,"Held PageUp repeatedly resets climb");
        if(testFrame==21)requireTower(g_towerRobot.progress<=restartedProgress+.0001f,"PageUp did not restart active climb");
    }
    requireTower(glGetError()==GL_NO_ERROR,"OpenGL error in tower render");
    if(testFrame==24){
        int w,h;glfwGetFramebufferSize(window,&w,&h);std::vector<unsigned char> pixels(w*h*3);
        glPixelStorei(GL_PACK_ALIGNMENT,1);glReadBuffer(GL_BACK);glFinish();
        glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
        std::ofstream out(testOutput,std::ios::binary);out<<"P6\n"<<w<<" "<<h<<"\n255\n";
        for(int y=h-1;y>=0;--y)out.write((char*)pixels.data()+y*w*3,w*3);
        requireTower((bool)out,"Capture write failed");
        std::cout<<"PASS render / controls: "<<testView<<"\n";glfwSetWindowShouldClose(window,true);
    }
    glfwSwapBuffers(window);++testFrame;
}
#define main testProjectMain
#define glfwGetTime testTime
#define glfwGetKey testKey
#define glfwCreateWindow testWindow
#define glfwSwapBuffers testSwap
#define glfwGetCursorPos testCursor
#include "../Main.cpp"
#undef main
#undef glfwGetTime
#undef glfwGetKey
#undef glfwCreateWindow
#undef glfwSwapBuffers
#undef glfwGetCursorPos
static void checkSteeringFrame(){
    if((testView=="boat-right"||testView=="boat-left")&&testFrame==10){
        float sign=testView=="boat-right"?-1.f:1.f;
        requireTower(g_boatPilotMode&&g_boatState.rudderAngle*sign>0,"Boat A/D rudder direction is reversed");
        requireTower((g_boatState.heading+22.f)*sign>0&&g_boatState.yawVelocity*sign>0,
                     "Boat A/D heading turns opposite to requested direction");
    }
    if(testView=="head-controls"&&testFrame==5)requireTower(g_robotHeadYaw<15,"Key 2 turns seated head left");
    if(testView=="head-controls"&&testFrame==11)requireTower(fabsf(g_robotHeadYaw-15)<.01f,"Key 1 does not turn seated head left");
}
static void prepareTowerFrame(){
    if(testFrame!=1)return;
    if(testView=="stairs-pov"||testView=="cutaway")g_towerRobot.progress=(float)g_towerRobot.spiralBegin+35.4f;
    if(testView=="top-pov"||testView=="top-switch")g_towerRobot.progress=(float)g_towerRobot.route.size()-1;
    if(testView=="entry"){
        g_towerViewMode=0;g_towerRobot.running=false;camera.setPreset(VIEW_TOWER);
        camera.isTransitioning=false;camera.target={-6,g_lighthouseBaseHeight+2,-110};
        camera.distance=11;camera.yaw=8;camera.pitch=12;
    }
    if(testView=="tower-model"){
        g_towerViewMode=0;g_towerRobot.running=false;g_lighthouseCutaway=true;camera.setPreset(VIEW_TOWER);
        camera.isTransitioning=false;camera.target={-6,g_lighthouseBaseHeight+9,-110};
        camera.distance=31;camera.yaw=8;camera.pitch=5;
    }
}
static void checkRoute(){
    float base=getCoastalTerrainElevation(-6,-110);
    for(int ring=1;ring<=32;++ring)for(int i=0;i<256;++i){
        float a=i*6.283185307f/256,r=3.2f*ring/32;
        base=glm::max(base,getCoastalTerrainElevation(-6+r*cosf(a),-110+r*sinf(a)));
    }
    base+=.18f;TowerClimbState tour;tour.reset(base);
    auto manual=tour;manual.moveManually(-1,1);requireTower(manual.progress==0,"Manual lower bound failed");
    manual.moveManually(1,.1f);float moved=manual.progress;
    requireTower(moved>0,"Manual forward movement failed");
    manual.moveManually(0,.1f);requireTower(manual.progress==moved,"Manual release failed");
    manual.moveManually(-1,.1f);requireTower(manual.progress==0,"Manual reverse movement failed");
    manual.progress=(float)manual.route.size()-1;manual.moveManually(1,.1f);
    requireTower(manual.finished(),"Manual upper bound failed");
    std::cout<<"Entrance probe: base "<<base<<", steps "<<tour.access.approachSteps<<", rise "<<tour.access.approachRise<<"\n";
    requireTower(tour.spiralEnd-tour.spiralBegin+1==72,"Not all 72 treads traversed");
    requireTower(tour.access.approachRise>=.10f&&tour.access.approachRise<=.171f,"Entrance risers are not usable");
    requireTower(LighthouseAccess::floorOpening>LighthouseAccess::stairOuter,"Floors block stair shaft");
    for(int j=0;j<tour.access.approachSteps;++j)for(float side:{-.90f,0.f,.90f}){
        auto p=tour.access.world({side,tour.access.approachTop(j),tour.access.landingEdge+(j+.5f)*tour.access.going});
        requireTower(getCoastalTerrainElevation(p.x,p.z)<=p.y+.02f,"Terrain protrudes through entrance steps");
    }
    for(int i=0;i<72;++i){
        auto p=tour.point(tour.spiralBegin+i);
        requireTower(glm::distance(p,tour.access.spiralPoint(i))<.00001f,"Route misses physical tread");
        requireTower(glm::length(glm::vec2(p.x,p.z))+.17f<tour.access.stairOuter,"Foot leaves tread");
    }
    for(float fps:{30.f,60.f,144.f}){
        auto state=tour;state.running=true;
        for(int i=0;i<(int)ceilf(20*fps);++i)state.update(1/fps);
        requireTower(fabsf(state.progress-20/state.stepSeconds)<.004f,"Climb speed depends on frame rate");
        state.paused=true;float before=state.progress;state.update(5);
        requireTower(state.progress==before,"Paused robot moves");
        state.paused=false;state.update(1000);
        requireTower(state.finished()&&!state.running,"Climb does not stop at gallery");
        requireTower(fabsf(state.localPosition().y-14.21f)<.001f,"Incorrect arrival floor");
    }
    for(float p=0;p<tour.route.size()-1;p+=.05f){
        tour.progress=p;auto pose=tour.pose();auto eye=tour.eye();
        requireTower(std::isfinite(eye.x)&&std::isfinite(eye.y)&&std::isfinite(eye.z),"Nonfinite POV");
        for(int leg=0;leg<2;++leg)requireTower(std::isfinite(pose.legs.hipPitch[leg])&&
            pose.legs.kneePitch[leg]>=0&&pose.legs.kneePitch[leg]<=180,"Invalid IK pose");
        requireTower(eye.y>tour.position().y+1.6f,"Eye intersects stair floor");
    }
    tour.progress=(float)tour.route.size()-1-.0001f;float before=tour.heading();
    tour.progress=(float)tour.route.size()-1;
    requireTower(fabsf(std::remainder(before-tour.heading(),360.f))<.2f,"Camera snaps on gallery arrival");
    std::cout<<"PASS route: "<<tour.access.approachSteps<<" approach steps, rise "<<tour.access.approachRise
        <<" m; 72 matched spiral treads; duration "<<(tour.route.size()-1)*tour.stepSeconds
        <<" s; 30/60/144 FPS, pause, arrival and IK\n";
}
int main(int argc,char** argv){try{
    checkRoute();if(argc<3)return 0;testView=argv[1];testOutput=argv[2];g_dayTime=12;g_dayPaused=true;
    g_objectGuide.visible=false;return testProjectMain();
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
