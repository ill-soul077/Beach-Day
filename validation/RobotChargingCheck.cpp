#include "Scene.h"
#include <iostream>
#include <stdexcept>

static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
    BeachCircuit circuit;
    auto station=beachChargingStationTransform(circuit);
    BeachRobotState robot;
    robot.pos=glm::vec3(station*glm::vec4(.20f,0,.65f,1));
    robot.pos.y=getCoastalTerrainElevation(robot.pos.x,robot.pos.z);
    robot.heading=glm::degrees(atan2f(station[2].x,station[2].z));
    robot.active=true;
    auto pivot=robot;glm::vec3 pivotPosition=pivot.pos;float pivotHeading=pivot.heading;
    std::vector<SolidObstacle> noObstacles;
    updateBeachRobot(pivot,0,1,.1f,noObstacles);
    check(pivot.heading<pivotHeading && pivot.turnRate<0,"Robot D/right pivot turns the wrong way");
    auto facing=[](float heading){float a=glm::radians(heading);return glm::vec3(sinf(a),0,cosf(a));};
    check(glm::dot(facing(pivot.heading),glm::cross(facing(pivotHeading),glm::vec3(0,1,0)))>0,
          "Robot D is not right from behind the robot");
    check(pivot.pos==pivotPosition && pivot.phase!=0 && pivot.gaitBlend>0,"Robot pivot slides or has no foot animation");
    updateBeachRobot(pivot,0,0,.2f,noObstacles);
    check(pivot.turnRate==0,"Robot keeps rotating after turn release");
    float settledHeading=pivot.heading;updateBeachRobot(pivot,0,0,.2f,noObstacles);
    check(pivot.heading==settledHeading,"Robot idle yaw drifts");
    auto left=robot;updateBeachRobot(left,0,-1,.1f,noObstacles);
    check(left.heading>pivotHeading,"Robot A/left pivot turns the wrong way");
    auto reversePivot=robot;updateBeachRobot(reversePivot,-1,1,.1f,noObstacles);
    check(reversePivot.heading<pivotHeading,"Robot backward walking reverses its pivot controls like vehicle steering");
    BeachRobotState turnReference;
    for(float dt:{1/60.f,1/30.f,.05f,.1f}){
        auto r=robot;
        for(int i=0;i<(int)roundf(.6f/dt);++i)updateBeachRobot(r,0,1,dt,noObstacles);
        if(dt==1/60.f)turnReference=r;
        check(fabsf(r.heading-turnReference.heading)<.15f,"Robot pivot depends on frame rate");
    }
    std::vector<SolidObstacle> obstacles={SolidObstacle::circle(robot.pos,.46f),
        SolidObstacle::box(glm::translate(station,{1.23f,0,.65f}),{.30f,.26f})};
    check(canConnectRobotCharger(robot,circuit,obstacles),"Valid robot docking pose rejected");
    robot.speed=1;check(!canConnectRobotCharger(robot,circuit,obstacles),"Moving robot can connect");robot.speed=0;
    auto far=robot;far.pos=circuit.sample(0);check(!canConnectRobotCharger(far,circuit,obstacles),"Remote robot can connect");
    auto blocked=obstacles;blocked.push_back(SolidObstacle::circle(robot.pos,.2f));
    check(!canConnectRobotCharger(robot,circuit,blocked),"Blocked docking pose accepted");
    for(float dt:{1/30.f,1/60.f,.25f}){
        auto charging=robot;charging.charging=true;charging.batteryPercent=10;
        glm::vec3 before=charging.pos;float heading=charging.heading;
        for(int i=0;i<(int)roundf(30/dt);++i)updateBeachRobot(charging,1,1,dt,obstacles);
        check(charging.batteryPercent==100,"Charging depends on frame rate or exceeds 100%");
        check(charging.pos==before&&charging.heading==heading&&charging.speed==0,"Connected robot can move or turn");
        charging.charging=false;
        for(int i=0;i<15;++i)updateBeachRobot(charging,-1,0,1/60.f,obstacles);
        check(glm::distance(charging.pos,before)>.03f,"Disconnect does not restore movement");
        check(charging.batteryPercent<100,"Walking does not drain battery");
    }
    auto empty=robot;empty.batteryPercent=0;
    updateBeachRobot(empty,1,1,.2f,obstacles);
    check(empty.pos==robot.pos&&empty.heading==robot.heading,"Empty robot can move");
    std::cout<<"PASS: robot foot-pivot steering, left/right/reverse controls, turn release, frame-rate consistency, docking, recharge, movement lock and battery drain\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
