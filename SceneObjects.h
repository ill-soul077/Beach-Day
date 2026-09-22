#ifndef SCENE_OBJECTS_H
#define SCENE_OBJECTS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "MeshPrimitives.h"
#include "shaderClass.h"

// ============================================================================
// HELPER DRAWING FUNCTIONS FOR CUSTOM PRIMITIVES
// All objects are rendered purely through these two primitives:
// 1. CubeMesh (stitched 6 square faces)
// 2. SphereMesh (parametric procedural latitude/longitude rings)
// ZERO built-in GLUT or GLU objects are used!
// ============================================================================

inline void drawTransformedCube(
    Shader& shader,
    const CubeMesh& cube,
    const glm::mat4& model,
    const glm::vec3& color,
    float shininess = 32.0f,
    float specularStrength = 0.25f,
    int unlit = 0)
{
    GLuint modelLoc = glGetUniformLocation(shader.ID, "model");
    GLuint colorLoc = glGetUniformLocation(shader.ID, "objectColor");
    GLuint specLoc  = glGetUniformLocation(shader.ID, "specularStrength");
    GLuint shinLoc  = glGetUniformLocation(shader.ID, "shininess");
    GLuint unlitLoc = glGetUniformLocation(shader.ID, "unlit");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glUniform1f(specLoc, specularStrength);
    glUniform1f(shinLoc, shininess);
    glUniform1i(unlitLoc, unlit);

    cube.draw();
}

inline void drawTransformedSphere(
    Shader& shader,
    const SphereMesh& sphere,
    const glm::mat4& model,
    const glm::vec3& color,
    float shininess = 48.0f,
    float specularStrength = 0.35f,
    int unlit = 0)
{
    GLuint modelLoc = glGetUniformLocation(shader.ID, "model");
    GLuint colorLoc = glGetUniformLocation(shader.ID, "objectColor");
    GLuint specLoc  = glGetUniformLocation(shader.ID, "specularStrength");
    GLuint shinLoc  = glGetUniformLocation(shader.ID, "shininess");
    GLuint unlitLoc = glGetUniformLocation(shader.ID, "unlit");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glUniform1f(specLoc, specularStrength);
    glUniform1f(shinLoc, shininess);
    glUniform1i(unlitLoc, unlit);

    sphere.draw();
}

inline void drawTransformedGore(
    Shader& shader,
    const CanopyGoreMesh& gore,
    const glm::mat4& model,
    const glm::vec3& color,
    float shininess = 24.0f,
    float specularStrength = 0.20f,
    int unlit = 0)
{
    GLuint modelLoc = glGetUniformLocation(shader.ID, "model");
    GLuint colorLoc = glGetUniformLocation(shader.ID, "objectColor");
    GLuint specLoc  = glGetUniformLocation(shader.ID, "specularStrength");
    GLuint shinLoc  = glGetUniformLocation(shader.ID, "shininess");
    GLuint unlitLoc = glGetUniformLocation(shader.ID, "unlit");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glUniform1f(specLoc, specularStrength);
    glUniform1f(shinLoc, shininess);
    glUniform1i(unlitLoc, unlit);

    gore.draw();
}

// ============================================================================
// OBJECT 1: ROBOT (SEATED UNDER UMBRELLA)
// Constructed hierarchically with cubes and spheres.
// Fully editable parameters: head turn, arm gestures, leg seating angles.
// ============================================================================
inline void drawRobot(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform,
    float headYaw = 10.0f,
    float headPitch = 0.0f,
    float leftArmPitch = -25.0f,
    float rightArmPitch = 20.0f,
    float legBendAngle = 80.0f)
{
    // Color Palette
    glm::vec3 metalBodyColor   = glm::vec3(0.72f, 0.76f, 0.82f); // Brushed steel
    glm::vec3 darkJointColor   = glm::vec3(0.28f, 0.30f, 0.35f); // Dark carbon joints
    glm::vec3 visorColor       = glm::vec3(0.0f, 0.85f, 1.0f);   // Glowing cyan visor
    glm::vec3 chestDialColor   = glm::vec3(1.0f, 0.65f, 0.1f);   // Amber core indicator
    glm::vec3 accentYellow     = glm::vec3(0.95f, 0.80f, 0.20f); // Hazard / trim strip

    // 1. PELVIS / BASE (Center of Robot Hierarchy)
    // Keep the pelvis narrower than the shoulders, as on a human-sized service robot.
    glm::mat4 pelvisMat = glm::scale(baseTransform, glm::vec3(0.48f, 0.20f, 0.32f));
    drawTransformedCube(shader, cube, pelvisMat, darkJointColor);

    glm::mat4 waistMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.20f, 0.0f));
    waistMat = glm::scale(waistMat, glm::vec3(0.28f, 0.18f, 0.25f));
    drawTransformedSphere(shader, sphere, waistMat, darkJointColor);

    // 2. TORSO / CHEST
    glm::mat4 torsoMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.47f, 0.0f));
    glm::mat4 torsoScale = glm::scale(torsoMat, glm::vec3(0.66f, 0.52f, 0.36f));
    drawTransformedCube(shader, cube, torsoScale, metalBodyColor);

    // A slightly wider shoulder yoke gives the torso a believable load-bearing shape.
    glm::mat4 shoulderYoke = glm::translate(torsoMat, glm::vec3(0.0f, 0.22f, 0.0f));
    shoulderYoke = glm::scale(shoulderYoke, glm::vec3(0.78f, 0.10f, 0.34f));
    drawTransformedCube(shader, cube, shoulderYoke, metalBodyColor);

    // Chest Front Indicator Screen / Panel
    glm::mat4 screenMat = glm::translate(torsoMat, glm::vec3(0.0f, 0.05f, 0.205f));
    screenMat = glm::scale(screenMat, glm::vec3(0.35f, 0.25f, 0.02f));
    drawTransformedCube(shader, cube, screenMat, darkJointColor);

    // Chest Arc Core / Dial (glowing sphere)
    glm::mat4 coreMat = glm::translate(torsoMat, glm::vec3(0.0f, 0.05f, 0.215f));
    coreMat = glm::scale(coreMat, glm::vec3(0.12f, 0.12f, 0.04f));
    drawTransformedSphere(shader, sphere, coreMat, chestDialColor, 64.0f, 0.5f, 1);

    // 3. NECK & HEAD
    glm::mat4 neckMat = glm::translate(torsoMat, glm::vec3(0.0f, 0.34f, 0.0f));
    glm::mat4 neckScale = glm::scale(neckMat, glm::vec3(0.15f, 0.14f, 0.15f));
    drawTransformedSphere(shader, sphere, neckScale, darkJointColor);

    // Head (rotates with headYaw & headPitch)
    glm::mat4 headMat = glm::translate(neckMat, glm::vec3(0.0f, 0.21f, 0.0f));
    headMat = glm::rotate(headMat, glm::radians(headYaw), glm::vec3(0.0f, 1.0f, 0.0f));
    headMat = glm::rotate(headMat, glm::radians(headPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 headScale = glm::scale(headMat, glm::vec3(0.40f, 0.34f, 0.34f));
    drawTransformedCube(shader, cube, headScale, metalBodyColor);

    // Visor Eyes (Cyan glowing bar)
    glm::mat4 visorMat = glm::translate(headMat, glm::vec3(0.0f, 0.04f, 0.19f));
    visorMat = glm::scale(visorMat, glm::vec3(0.32f, 0.09f, 0.04f));
    drawTransformedCube(shader, cube, visorMat, visorColor, 64.0f, 0.5f, 1);

    // Antenna Mast (Rod + Ball on head)
    glm::mat4 antPole = glm::translate(headMat, glm::vec3(0.10f, 0.25f, 0.0f));
    antPole = glm::scale(antPole, glm::vec3(0.03f, 0.16f, 0.03f));
    drawTransformedCube(shader, cube, antPole, darkJointColor);

    glm::mat4 antTip = glm::translate(headMat, glm::vec3(0.10f, 0.35f, 0.0f));
    antTip = glm::scale(antTip, glm::vec3(0.08f, 0.08f, 0.08f));
    drawTransformedSphere(shader, sphere, antTip, visorColor, 64.0f, 0.6f, 1);

    // Ear Bolts (Left & Right)
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 earMat = glm::translate(headMat, glm::vec3(side * 0.22f, 0.0f, 0.0f));
        earMat = glm::scale(earMat, glm::vec3(0.06f, 0.12f, 0.12f));
        drawTransformedSphere(shader, sphere, earMat, darkJointColor);
    }

    // 4. ARMS (Left and Right)
    for (int side = -1; side <= 1; side += 2)
    {
        float armPitch = (side == -1) ? leftArmPitch : rightArmPitch;

        // Shoulder Joint
        glm::mat4 shoulderMat = glm::translate(torsoMat, glm::vec3(side * 0.40f, 0.20f, 0.0f));
        glm::mat4 shoulderBall = glm::scale(shoulderMat, glm::vec3(0.16f, 0.16f, 0.16f));
        drawTransformedSphere(shader, sphere, shoulderBall, darkJointColor);

        // Upper Arm
        glm::mat4 upperArm = glm::rotate(shoulderMat, glm::radians(armPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 upperArmSeg = glm::translate(upperArm, glm::vec3(0.0f, -0.18f, 0.0f));
        glm::mat4 upperArmScale = glm::scale(upperArmSeg, glm::vec3(0.12f, 0.28f, 0.12f));
        drawTransformedCube(shader, cube, upperArmScale, metalBodyColor);

        // Elbow Joint
        glm::mat4 elbowMat = glm::translate(upperArm, glm::vec3(0.0f, -0.34f, 0.0f));
        glm::mat4 elbowBall = glm::scale(elbowMat, glm::vec3(0.12f, 0.12f, 0.12f));
        drawTransformedSphere(shader, sphere, elbowBall, darkJointColor);

        // Forearm (bent forward slightly)
        glm::mat4 foreArm = glm::rotate(elbowMat, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 foreArmSeg = glm::translate(foreArm, glm::vec3(0.0f, -0.17f, 0.0f));
        glm::mat4 foreArmScale = glm::scale(foreArmSeg, glm::vec3(0.11f, 0.26f, 0.11f));
        drawTransformedCube(shader, cube, foreArmScale, metalBodyColor);

        // Hand / Gripper Clamp
        glm::mat4 handMat = glm::translate(foreArm, glm::vec3(0.0f, -0.32f, 0.025f));
        glm::mat4 handScale = glm::scale(handMat, glm::vec3(0.13f, 0.10f, 0.16f));
        drawTransformedCube(shader, cube, handScale, darkJointColor);
    }

    // 5. LEGS (Seated Pose: Thighs extended forward, knees bent downward)
    for (int side = -1; side <= 1; side += 2)
    {
        // Hip Joint
        glm::mat4 hipMat = glm::translate(baseTransform, glm::vec3(side * 0.16f, -0.07f, 0.04f));
        glm::mat4 hipBall = glm::scale(hipMat, glm::vec3(0.14f, 0.14f, 0.14f));
        drawTransformedSphere(shader, sphere, hipBall, darkJointColor);

        // Thigh (Rotated 80 degrees forward to sit on chair)
        glm::mat4 thighMat = glm::rotate(hipMat, glm::radians(legBendAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 thighSeg = glm::translate(thighMat, glm::vec3(0.0f, -0.20f, 0.0f));
        glm::mat4 thighScale = glm::scale(thighSeg, glm::vec3(0.14f, 0.32f, 0.14f));
        drawTransformedCube(shader, cube, thighScale, metalBodyColor);

        // Knee Joint
        glm::mat4 kneeMat = glm::translate(thighMat, glm::vec3(0.0f, -0.39f, 0.0f));
        glm::mat4 kneeBall = glm::scale(kneeMat, glm::vec3(0.13f, 0.13f, 0.13f));
        drawTransformedSphere(shader, sphere, kneeBall, darkJointColor);

        // Shin / Lower Leg (Bent down 80 degrees relative to thigh)
        glm::mat4 shinMat = glm::rotate(kneeMat, glm::radians(-legBendAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 shinSeg = glm::translate(shinMat, glm::vec3(0.0f, -0.20f, 0.0f));
        glm::mat4 shinScale = glm::scale(shinSeg, glm::vec3(0.13f, 0.32f, 0.13f));
        drawTransformedCube(shader, cube, shinScale, metalBodyColor);

        // Ankle / Foot
        glm::mat4 footMat = glm::translate(shinMat, glm::vec3(0.0f, -0.39f, 0.09f));
        glm::mat4 footScale = glm::scale(footMat, glm::vec3(0.17f, 0.10f, 0.28f));
        drawTransformedCube(shader, cube, footScale, darkJointColor);
    }
}

// ============================================================================
// OBJECT 2: BEACH UMBRELLA
// Hierarchical modeling: Tilting the pole automatically tilts the canopy!
// Canopy is crafted from alternating coral red and white gore panels.
// ============================================================================
inline void drawUmbrella(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const CanopyGoreMesh& gore,
    const glm::mat4& baseTransform,
    float poleTiltAngle = 12.0f,
    float poleHeight = 3.2f,
    float canopyScale = 1.0f)
{
    glm::vec3 poleColor   = glm::vec3(0.85f, 0.85f, 0.88f); // White/silver aluminum pole
    glm::vec3 ribColor    = glm::vec3(0.35f, 0.35f, 0.38f); // Steel canopy ribs
    glm::vec3 stripeRed   = glm::vec3(0.92f, 0.22f, 0.18f); // Summer coral red
    glm::vec3 stripeWhite = glm::vec3(0.97f, 0.97f, 0.97f); // Bright clean white
    glm::vec3 tipColor    = glm::vec3(0.95f, 0.80f, 0.15f); // Golden finial tip

    // 1. BASE ANCHOR (Flat cube plate in sand)
    glm::mat4 anchorMat = glm::scale(baseTransform, glm::vec3(0.6f, 0.06f, 0.6f));
    drawTransformedCube(shader, cube, anchorMat, poleColor);

    // 2. LOWER POLE (Vertical into the sand)
    float lowerHeight = 1.0f;
    glm::mat4 lowerPoleMat = glm::translate(baseTransform, glm::vec3(0.0f, lowerHeight * 0.5f, 0.0f));
    lowerPoleMat = glm::scale(lowerPoleMat, glm::vec3(0.08f, lowerHeight, 0.08f));
    drawTransformedCube(shader, cube, lowerPoleMat, poleColor);

    // 3. TILT JOINT (Hierarchical Node)
    glm::mat4 tiltNode = glm::translate(baseTransform, glm::vec3(0.0f, lowerHeight, 0.0f));
    tiltNode = glm::rotate(tiltNode, glm::radians(poleTiltAngle), glm::vec3(0.0f, 0.0f, 1.0f));

    // Tilt Joint Sphere
    glm::mat4 jointSphere = glm::scale(tiltNode, glm::vec3(0.14f, 0.14f, 0.14f));
    drawTransformedSphere(shader, sphere, jointSphere, poleColor);

    // 4. UPPER POLE (Extends along tilted axis)
    float upperHeight = poleHeight - lowerHeight;
    glm::mat4 upperPoleMat = glm::translate(tiltNode, glm::vec3(0.0f, upperHeight * 0.5f, 0.0f));
    upperPoleMat = glm::scale(upperPoleMat, glm::vec3(0.075f, upperHeight, 0.075f));
    drawTransformedCube(shader, cube, upperPoleMat, poleColor);

    // 5. CANOPY CROWN / APEX (Top of pole)
    glm::mat4 canopyCrown = glm::translate(tiltNode, glm::vec3(0.0f, upperHeight, 0.0f));
    canopyCrown = glm::scale(canopyCrown, glm::vec3(canopyScale));

    // Top Finial Ball & Cap
    glm::mat4 finialCap = glm::translate(canopyCrown, glm::vec3(0.0f, 0.12f, 0.0f));
    finialCap = glm::scale(finialCap, glm::vec3(0.22f, 0.16f, 0.22f));
    drawTransformedSphere(shader, sphere, finialCap, tipColor);

    glm::mat4 finialTip = glm::translate(canopyCrown, glm::vec3(0.0f, 0.22f, 0.0f));
    finialTip = glm::scale(finialTip, glm::vec3(0.10f, 0.14f, 0.10f));
    drawTransformedSphere(shader, sphere, finialTip, tipColor);

    // 6. SEAMLESS CANOPY GORES (12 sectors alternating Coral Red & Crisp White)
    // Seamless edge-to-edge alignment with zero star overlap and zero gaps
    for (int i = 0; i < 12; ++i)
    {
        float angle = (float)i * 30.0f;
        glm::vec3 panelColor = (i % 2 == 0) ? stripeRed : stripeWhite;

        glm::mat4 goreMat = glm::rotate(canopyCrown, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        drawTransformedGore(shader, gore, goreMat, panelColor);
    }

    // 7. CLEAN UNDERSIDE HUB (Neat small collar ring directly beneath canopy apex, no protruding wires)
    glm::mat4 collarMat = glm::translate(canopyCrown, glm::vec3(0.0f, -0.06f, 0.0f));
    collarMat = glm::scale(collarMat, glm::vec3(0.20f, 0.12f, 0.20f));
    drawTransformedSphere(shader, sphere, collarMat, poleColor);
}

// ============================================================================
// OBJECT 3: BEACH CHAIR / LOUNGER
// Placed beneath the umbrella for the robot to sit or recline on.
// ============================================================================
inline void drawBeachChair(
    Shader& shader,
    const CubeMesh& cube,
    const glm::mat4& baseTransform)
{
    glm::vec3 woodColor   = glm::vec3(0.76f, 0.54f, 0.35f); // Beach teak wood
    glm::vec3 fabricColor = glm::vec3(0.20f, 0.65f, 0.90f); // Bright azure ocean blue
    glm::vec3 whiteStripe = glm::vec3(0.95f, 0.95f, 0.95f);

    // 1. CHAIR LEGS (4 vertical wooden supports)
    // A real lounger is wider than its occupant and long enough to support the thighs.
    float legX = 0.52f;
    float legZ = 0.66f;
    float legHeight = 0.44f;
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
        {
            glm::mat4 legMat = glm::translate(baseTransform, glm::vec3(sx * legX, legHeight * 0.5f, sz * legZ));
            legMat = glm::rotate(legMat, glm::radians(sz * 7.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            legMat = glm::scale(legMat, glm::vec3(0.075f, legHeight, 0.075f));
            drawTransformedCube(shader, cube, legMat, woodColor);
        }
    }

    // 2. HORIZONTAL SEAT BASE FRAME
    glm::mat4 railLeft = glm::translate(baseTransform, glm::vec3(-legX, legHeight, 0.0f));
    railLeft = glm::scale(railLeft, glm::vec3(0.075f, 0.075f, legZ * 2.15f));
    drawTransformedCube(shader, cube, railLeft, woodColor);

    glm::mat4 railRight = glm::translate(baseTransform, glm::vec3(legX, legHeight, 0.0f));
    railRight = glm::scale(railRight, glm::vec3(0.075f, 0.075f, legZ * 2.15f));
    drawTransformedCube(shader, cube, railRight, woodColor);

    // 3. SEAT CUSHION / SLATS
    int slats = 9;
    for (int i = 0; i < slats; ++i)
    {
        float zPos = -legZ + (float)i * ((legZ * 2.0f) / (float)(slats - 1));
        glm::vec3 slatColor = (i % 2 == 0) ? fabricColor : whiteStripe;

        glm::mat4 slatMat = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.035f, zPos));
        slatMat = glm::scale(slatMat, glm::vec3(legX * 1.88f, 0.045f, 0.13f));
        drawTransformedCube(shader, cube, slatMat, slatColor);
    }

    // 4. RECLINING BACKREST (Angled at 35 degrees backwards)
    glm::mat4 backPivot = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.05f, -legZ));
    backPivot = glm::rotate(backPivot, glm::radians(-28.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Backrest side rails
    for (int sx = -1; sx <= 1; sx += 2)
    {
        glm::mat4 backRail = glm::translate(backPivot, glm::vec3(sx * legX, 0.52f, 0.0f));
        backRail = glm::scale(backRail, glm::vec3(0.075f, 1.04f, 0.075f));
        drawTransformedCube(shader, cube, backRail, woodColor);
    }

    // Backrest slats
    for (int i = 0; i < 7; ++i)
    {
        float yPos = 0.14f + (float)i * 0.14f;
        glm::vec3 slatColor = (i % 2 == 0) ? fabricColor : whiteStripe;
        glm::mat4 bSlat = glm::translate(backPivot, glm::vec3(0.0f, yPos, 0.0f));
        bSlat = glm::scale(bSlat, glm::vec3(legX * 1.88f, 0.10f, 0.04f));
        drawTransformedCube(shader, cube, bSlat, slatColor);
    }

    // Armrests sit around elbow height and also brace the reclining back.
    for (int sx = -1; sx <= 1; sx += 2)
    {
        glm::mat4 armPost = glm::translate(baseTransform, glm::vec3(sx * 0.56f, 0.66f, 0.18f));
        armPost = glm::scale(armPost, glm::vec3(0.06f, 0.40f, 0.06f));
        drawTransformedCube(shader, cube, armPost, woodColor);

        glm::mat4 armRest = glm::translate(baseTransform, glm::vec3(sx * 0.56f, 0.86f, -0.12f));
        armRest = glm::scale(armRest, glm::vec3(0.10f, 0.055f, 0.68f));
        drawTransformedCube(shader, cube, armRest, woodColor);
    }
}

// ============================================================================
// OBJECT 4: SAILBOAT
// Realistic V-hull with tapered bow and stern, layered hull planks,
// gunwales, deck, cabin, mast, boom, and properly angled sails.
// ============================================================================
inline void drawBoat(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform)
{
    glm::vec3 hullNavy   = glm::vec3(0.12f, 0.22f, 0.42f);
    glm::vec3 hullDark   = glm::vec3(0.08f, 0.15f, 0.30f);
    glm::vec3 deckWhite  = glm::vec3(0.93f, 0.93f, 0.95f);
    glm::vec3 deckTan    = glm::vec3(0.78f, 0.62f, 0.40f);
    glm::vec3 mastBrown  = glm::vec3(0.55f, 0.35f, 0.20f);
    glm::vec3 sailWhite  = glm::vec3(0.97f, 0.97f, 0.94f);
    glm::vec3 redStripe  = glm::vec3(0.85f, 0.12f, 0.12f);
    glm::vec3 railWhite  = glm::vec3(0.90f, 0.90f, 0.92f);

    // 1. ROUNDED DISPLACEMENT HULL. Overlapping ellipsoids give a continuous,
    // tapered silhouette; the lower form stays below the water like a real keel.
    glm::mat4 hullBody = glm::translate(baseTransform, glm::vec3(0.0f, 0.08f, 0.10f));
    hullBody = glm::scale(hullBody, glm::vec3(1.28f, 0.72f, 3.75f));
    drawTransformedSphere(shader, sphere, hullBody, hullNavy);

    glm::mat4 keelMat = glm::translate(baseTransform, glm::vec3(0.0f, -0.18f, -0.05f));
    keelMat = glm::scale(keelMat, glm::vec3(0.42f, 0.48f, 2.75f));
    drawTransformedSphere(shader, sphere, keelMat, hullDark);

    // Waterline accents remain on the sides instead of cutting through the hull.
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 stripeMat = glm::translate(baseTransform, glm::vec3(side * 0.615f, 0.13f, 0.02f));
        stripeMat = glm::scale(stripeMat, glm::vec3(0.035f, 0.055f, 3.18f));
        drawTransformedCube(shader, cube, stripeMat, redStripe);
    }

    // Bowsprit tip
    glm::mat4 bowTip = glm::translate(baseTransform, glm::vec3(0.0f, 0.34f, 2.05f));
    bowTip = glm::scale(bowTip, glm::vec3(0.055f, 0.055f, 0.52f));
    drawTransformedCube(shader, cube, bowTip, mastBrown);

    // 3. STERN (Flat transom)
    glm::mat4 sternMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.12f, -1.74f));
    sternMat = glm::scale(sternMat, glm::vec3(1.04f, 0.43f, 0.10f));
    drawTransformedCube(shader, cube, sternMat, hullNavy);

    // 4. DECK SURFACE (Flat planks on top)
    glm::mat4 deckMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.39f, -0.02f));
    deckMat = glm::scale(deckMat, glm::vec3(1.02f, 0.055f, 2.95f));
    drawTransformedCube(shader, cube, deckMat, deckTan);

    // Deck railing (thin top rails)
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 railMat = glm::translate(baseTransform, glm::vec3(side * 0.54f, 0.44f, -0.02f));
        railMat = glm::scale(railMat, glm::vec3(0.045f, 0.07f, 2.82f));
        drawTransformedCube(shader, cube, railMat, railWhite);
    }

    // 5. CABIN (Low-profile box on deck)
    glm::mat4 cabinMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.52f, -0.48f));
    cabinMat = glm::scale(cabinMat, glm::vec3(0.62f, 0.24f, 0.72f));
    drawTransformedCube(shader, cube, cabinMat, deckWhite);

    // Cabin roof
    glm::mat4 roofMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.65f, -0.48f));
    roofMat = glm::scale(roofMat, glm::vec3(0.70f, 0.035f, 0.80f));
    drawTransformedCube(shader, cube, roofMat, railWhite);

    // 6. VERTICAL MAST
    float mastHeight = 3.2f;
    glm::mat4 mastMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.35f + mastHeight * 0.5f, 0.40f));
    mastMat = glm::scale(mastMat, glm::vec3(0.07f, mastHeight, 0.07f));
    drawTransformedCube(shader, cube, mastMat, mastBrown);

    // Mast top ball
    glm::mat4 mastTop = glm::translate(baseTransform, glm::vec3(0.0f, 0.35f + mastHeight + 0.06f, 0.40f));
    mastTop = glm::scale(mastTop, glm::vec3(0.12f, 0.12f, 0.12f));
    drawTransformedSphere(shader, sphere, mastTop, mastBrown);

    // 7. BOOM (Horizontal spar for mainsail)
    glm::mat4 boomMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.85f, -0.40f));
    boomMat = glm::scale(boomMat, glm::vec3(0.05f, 0.05f, 1.6f));
    drawTransformedCube(shader, cube, boomMat, mastBrown);

    // 8. MAINSAIL (Proper tapered triangular sail)
    const int mainSailSlices = 18;
    for (int i = 0; i < mainSailSlices; ++i)
    {
        float t = ((float)i + 0.5f) / (float)mainSailSlices;
        float y = 0.90f + t * (mastHeight * 0.80f);
        float width = (1.0f - t) * 1.50f;

        glm::mat4 sailSlice = glm::translate(baseTransform, glm::vec3(0.02f, y, -0.40f + t * 0.50f));
        sailSlice = glm::scale(sailSlice, glm::vec3(0.018f, mastHeight * 0.82f / mainSailSlices + 0.012f, width));
        drawTransformedCube(shader, cube, sailSlice, sailWhite);
    }

    // 9. JIB / FORESAIL
    const int jibSlices = 14;
    for (int i = 0; i < jibSlices; ++i)
    {
        float t = ((float)i + 0.5f) / (float)jibSlices;
        float y = 0.55f + t * (mastHeight * 0.60f);
        float width = (1.0f - t) * 1.35f;
        float rearEdge = 0.43f;
        float centerZ = rearEdge + width * 0.5f;

        glm::mat4 jibSlice = glm::translate(baseTransform, glm::vec3(-0.02f, y, centerZ));
        jibSlice = glm::scale(jibSlice, glm::vec3(0.018f, mastHeight * 0.62f / jibSlices + 0.012f, width));
        drawTransformedCube(shader, cube, jibSlice, sailWhite);
    }
}

// ============================================================================
// OBJECT 5: PALM TREE
// Segmented curved trunk, visible coconut cluster, and realistic arching
// feathery palm fronds with tapered stems and pairs of angled leaflets.
// ============================================================================
inline void drawPalmTree(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform,
    float trunkCurveAngle = 3.2f)
{
    glm::vec3 barkColor    = glm::vec3(0.48f, 0.32f, 0.18f); // Palm bark brown
    glm::vec3 ringColor    = glm::vec3(0.38f, 0.24f, 0.12f); // Segment ring groove
    glm::vec3 coconutColor = glm::vec3(0.30f, 0.17f, 0.08f); // Coconuts
    glm::vec3 stemColor    = glm::vec3(0.28f, 0.52f, 0.16f); // Frond stem
    glm::vec3 leafGreen1   = glm::vec3(0.12f, 0.60f, 0.16f); // Lush tropical green
    glm::vec3 leafGreen2   = glm::vec3(0.15f, 0.68f, 0.20f); // Bright highlight green

    // 1. CURVED SEGMENTED TRUNK (Stacked and progressively tilted nodes)
    int trunkSegments = 11;
    float segHeight = 0.50f;
    glm::mat4 currentTransform = baseTransform;

    for (int i = 0; i < trunkSegments; ++i)
    {
        float taper = 1.0f - ((float)i / (float)trunkSegments) * 0.38f;
        glm::vec3 segCol = (i % 2 == 0) ? barkColor : ringColor;

        // Overlapping ellipsoids make a continuous, rounded, naturally tapered trunk.
        glm::mat4 segMat = glm::translate(currentTransform, glm::vec3(0.0f, segHeight * 0.5f, 0.0f));
        segMat = glm::scale(segMat, glm::vec3(0.43f * taper, segHeight * 1.12f, 0.43f * taper));
        drawTransformedSphere(shader, sphere, segMat, segCol);

        // A shallow collar suggests the old leaf-scar rings seen on real palms.
        if (i > 0)
        {
            glm::mat4 scarMat = glm::scale(currentTransform,
                glm::vec3(0.45f * taper, 0.075f, 0.45f * taper));
            drawTransformedSphere(shader, sphere, scarMat, ringColor);
        }

        // Advance to next segment with slight tilt for natural curve
        currentTransform = glm::translate(currentTransform, glm::vec3(0.0f, segHeight, 0.0f));
        currentTransform = glm::rotate(currentTransform, glm::radians(trunkCurveAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // 2. COCONUT CLUSTER (Nestled tightly around crown ring, fully visible)
    for (int i = 0; i < 5; ++i)
    {
        float angle = (float)i * 72.0f + 15.0f;
        glm::mat4 cocoMat = glm::rotate(currentTransform, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        cocoMat = glm::translate(cocoMat, glm::vec3(0.18f, -0.05f, 0.0f));
        cocoMat = glm::scale(cocoMat, glm::vec3(0.24f, 0.28f, 0.24f));
        drawTransformedSphere(shader, sphere, cocoMat, coconutColor);
    }

    // Crown Cap Sphere (organic transition between trunk and fronds)
    glm::mat4 capMat = glm::translate(currentTransform, glm::vec3(0.0f, 0.04f, 0.0f));
    capMat = glm::scale(capMat, glm::vec3(0.32f, 0.20f, 0.32f));
    drawTransformedSphere(shader, sphere, capMat, stemColor);

    // 3. REALISTIC DOWNWARD-ARCHING PALM FRONDS (two staggered age tiers)
    // Positive rotation around local X tilts +Z down towards -Y (towards the ground).
    struct FrondTierDef {
        int count;
        float angleOffset;
        float initialPitch;      // downward tilt at crown (degrees)
        float lengthScale;
        float curvatureMultiplier;
        glm::vec3 leafCol;
    };

    FrondTierDef tiers[2] = {
        // Young upper fronds reach upward before beginning their arch.
        { 7,   0.0f, -12.0f, 1.00f, 0.70f, leafGreen2 },
        // Mature lower fronds spread farther and hang more deeply.
        { 7,  25.7f,  10.0f, 1.18f, 1.10f, leafGreen1 }
    };

    for (int t = 0; t < 2; ++t)
    {
        const FrondTierDef& tier = tiers[t];
        float angleStep = 360.0f / (float)tier.count;

        for (int i = 0; i < tier.count; ++i)
        {
            float frondYaw = (float)i * angleStep + tier.angleOffset;
            glm::mat4 frondBase = glm::rotate(currentTransform, glm::radians(frondYaw), glm::vec3(0.0f, 1.0f, 0.0f));

            // Origin offset from trunk center so center stays open and coconuts remain visible
            glm::mat4 spineNode = glm::translate(frondBase, glm::vec3(0.0f, 0.08f, 0.22f));
            // Pitch downward from crown
            spineNode = glm::rotate(spineNode, glm::radians(tier.initialPitch), glm::vec3(1.0f, 0.0f, 0.0f));

            // Progressive POSITIVE angles to curve the spine DOWNWARD towards the ground
            float droopAngles[6] = {
                6.0f  * tier.curvatureMultiplier,
                10.0f * tier.curvatureMultiplier,
                14.0f * tier.curvatureMultiplier,
                18.0f * tier.curvatureMultiplier,
                22.0f * tier.curvatureMultiplier,
                24.0f * tier.curvatureMultiplier
            };

            for (int seg = 0; seg < 6; ++seg)
            {
                float segLen = 0.48f * tier.lengthScale;
                float segTaper = 1.0f - ((float)seg / 6.0f) * 0.70f; // 1.0 down to 0.3

                // Apply progressive droop to curve the spine downward
                spineNode = glm::rotate(spineNode, glm::radians(droopAngles[seg]), glm::vec3(1.0f, 0.0f, 0.0f));

                // 3a. Central Frond Rachis / Stem
                glm::mat4 stemMat = glm::translate(spineNode, glm::vec3(0.0f, 0.0f, segLen * 0.5f));
                stemMat = glm::scale(stemMat, glm::vec3(0.045f * segTaper, 0.030f * segTaper, segLen));
                drawTransformedCube(shader, cube, stemMat, stemColor);

                // 3b. Pairs of Angled Feathery Leaflets (Left & Right)
                if (seg >= 1) // skip bare stem base near crown
                {
                    // Leaflet span peaks in the middle (segments 2-3) and tapers toward the tip
                    float spanFactors[6] = { 0.0f, 0.65f, 1.0f, 0.90f, 0.65f, 0.35f };
                    float leafletSpan = 0.42f * spanFactors[seg] * tier.lengthScale;
                    float leafletWidth = 0.09f * spanFactors[seg] + 0.03f;

                    // Place 2 pairs of leaflets along this segment for lush density
                    for (int sub = 0; sub < 2; ++sub)
                    {
                        float subZ = segLen * (0.30f + 0.45f * (float)sub);

                        // Left Leaflet (Angled out +52 deg, tilted down -24 deg for drooping inverted V)
                        glm::mat4 leftLeaflet = glm::translate(spineNode, glm::vec3(0.0f, 0.0f, subZ));
                        leftLeaflet = glm::rotate(leftLeaflet, glm::radians(52.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                        leftLeaflet = glm::rotate(leftLeaflet, glm::radians(-24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                        leftLeaflet = glm::translate(leftLeaflet, glm::vec3(leafletSpan * 0.5f, 0.0f, 0.0f));
                        leftLeaflet = glm::scale(leftLeaflet, glm::vec3(leafletSpan, 0.012f, leafletWidth));
                        drawTransformedCube(shader, cube, leftLeaflet, tier.leafCol);

                        // Right Leaflet (Angled out -52 deg, tilted down -24 deg)
                        glm::mat4 rightLeaflet = glm::translate(spineNode, glm::vec3(0.0f, 0.0f, subZ));
                        rightLeaflet = glm::rotate(rightLeaflet, glm::radians(-52.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                        rightLeaflet = glm::rotate(rightLeaflet, glm::radians(24.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                        rightLeaflet = glm::translate(rightLeaflet, glm::vec3(-leafletSpan * 0.5f, 0.0f, 0.0f));
                        rightLeaflet = glm::scale(rightLeaflet, glm::vec3(leafletSpan, 0.012f, leafletWidth));
                        drawTransformedCube(shader, cube, rightLeaflet, tier.leafCol);
                    }
                }

                // Advance to the end of this segment
                spineNode = glm::translate(spineNode, glm::vec3(0.0f, 0.0f, segLen));
            }
        }
    }
}

// ============================================================================
// OBJECT 6: SUN
// Glowing celestial body rendered as unlit bright sphere along its day arc.
// ============================================================================
inline void drawSun(
    Shader& shader,
    const SphereMesh& sphere,
    const glm::vec3& sunPosition,
    float sunRadius = 2.0f,
    const glm::vec3& sunColor = glm::vec3(1.0f, 0.94f, 0.45f))
{
    glm::mat4 sunMat = glm::translate(glm::mat4(1.0f), sunPosition);
    sunMat = glm::scale(sunMat, glm::vec3(sunRadius * 2.0f));

    // Render as unlit glowing sphere
    drawTransformedSphere(shader, sphere, sunMat, sunColor, 1.0f, 0.0f, 1);
}

// ============================================================================
// OBJECT 7: SEA / WATER SURFACE
// Expansive ocean plane with coastal cyan/blue hues.
// ============================================================================
inline void drawSea(
    Shader& shader,
    const CubeMesh& cube,
    const glm::vec3& seaCenter = glm::vec3(0.0f, -0.1f, 15.0f),
    float width = 60.0f,
    float depth = 45.0f)
{
    glm::vec3 waterColor = glm::vec3(0.08f, 0.48f, 0.65f); // Tropical coastal ocean blue

    glm::mat4 seaMat = glm::translate(glm::mat4(1.0f), seaCenter);
    seaMat = glm::scale(seaMat, glm::vec3(width, 0.2f, depth));

    // Water has high shininess and specular highlight for sun reflection
    drawTransformedCube(shader, cube, seaMat, waterColor, 128.0f, 0.8f, 0);
}

// ============================================================================
// OBJECT 8: SAND BEACH / SHORELINE
// Ground plane where beach lounger, umbrella, and palm trees are situated.
// ============================================================================
inline void drawBeach(
    Shader& shader,
    const CubeMesh& cube,
    const glm::vec3& beachCenter = glm::vec3(0.0f, -0.1f, -8.0f),
    float width = 60.0f,
    float depth = 25.0f)
{
    glm::vec3 sandColor = glm::vec3(0.92f, 0.80f, 0.56f); // Warm coastal sand

    glm::mat4 sandMat = glm::translate(glm::mat4(1.0f), beachCenter);
    sandMat = glm::scale(sandMat, glm::vec3(width, 0.25f, depth));

    // Sand has low specular reflection (matte)
    drawTransformedCube(shader, cube, sandMat, sandColor, 8.0f, 0.08f, 0);
}

// ============================================================================
// OBJECT 9: COASTAL ROCKS / BOULDERS
// Irregular COMPOSITE rock shapes: each boulder is built from 2-4 overlapping
// cubes with different rotations and scales to create uneven, organic profiles.
// ============================================================================
inline void drawRocks(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere)
{
    glm::vec3 rockGray1  = glm::vec3(0.40f, 0.42f, 0.44f);
    glm::vec3 rockGray2  = glm::vec3(0.50f, 0.48f, 0.42f);
    glm::vec3 rockDark   = glm::vec3(0.30f, 0.32f, 0.34f);
    glm::vec3 rockLight  = glm::vec3(0.56f, 0.54f, 0.50f);
    glm::vec3 mossGreen  = glm::vec3(0.32f, 0.40f, 0.28f);

    // Each rock cluster is a composite of multiple primitives
    // LEFT CLUSTER (Moved out to X = -14.5 to -17.0)
    // Boulder 1 (Large main rock)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), glm::vec3(-15.5f, 0.30f, 1.5f));
        b = glm::rotate(b, glm::radians(32.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 b1 = glm::scale(b, glm::vec3(1.8f, 0.90f, 1.5f));
        drawTransformedSphere(shader, sphere, b1, rockGray1, 16.0f, 0.12f, 0);
        // Uneven bump on top
        glm::mat4 b2 = glm::translate(b, glm::vec3(0.25f, 0.42f, -0.15f));
        b2 = glm::rotate(b2, glm::radians(22.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b2 = glm::rotate(b2, glm::radians(-12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        b2 = glm::scale(b2, glm::vec3(1.0f, 0.55f, 0.85f));
        drawTransformedSphere(shader, sphere, b2, rockGray2, 16.0f, 0.12f, 0);
        // Side spur
        glm::mat4 b3 = glm::translate(b, glm::vec3(-0.65f, 0.10f, 0.30f));
        b3 = glm::rotate(b3, glm::radians(-28.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b3 = glm::rotate(b3, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        b3 = glm::scale(b3, glm::vec3(0.75f, 0.50f, 0.65f));
        drawTransformedSphere(shader, sphere, b3, rockDark, 16.0f, 0.10f, 0);
        // Moss patch
        glm::mat4 bm = glm::translate(b, glm::vec3(0.0f, 0.46f, 0.20f));
        bm = glm::scale(bm, glm::vec3(0.60f, 0.06f, 0.50f));
        drawTransformedSphere(shader, sphere, bm, mossGreen, 8.0f, 0.05f, 0);
    }

    // Boulder 2 (Medium, rounded with sphere composite)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), glm::vec3(-14.0f, 0.22f, 2.2f));
        b = glm::rotate(b, glm::radians(-18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 b1 = glm::scale(b, glm::vec3(1.1f, 0.65f, 0.95f));
        drawTransformedSphere(shader, sphere, b1, rockGray2, 16.0f, 0.12f, 0);
        glm::mat4 b2 = glm::translate(b, glm::vec3(0.15f, 0.20f, 0.0f));
        b2 = glm::rotate(b2, glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b2 = glm::scale(b2, glm::vec3(0.75f, 0.45f, 0.70f));
        drawTransformedSphere(shader, sphere, b2, rockLight, 16.0f, 0.10f, 0);
    }

    // Boulder 3 (Small craggy)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), glm::vec3(-16.8f, 0.25f, 0.6f));
        b = glm::rotate(b, glm::radians(55.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 b1 = glm::scale(b, glm::vec3(1.3f, 1.0f, 1.1f));
        drawTransformedSphere(shader, sphere, b1, rockDark, 16.0f, 0.10f, 0);
        glm::mat4 b2 = glm::translate(b, glm::vec3(0.40f, 0.35f, -0.25f));
        b2 = glm::rotate(b2, glm::radians(42.0f), glm::vec3(1.0f, 0.3f, 0.0f));
        b2 = glm::scale(b2, glm::vec3(0.65f, 0.55f, 0.50f));
        drawTransformedSphere(shader, sphere, b2, rockGray1, 16.0f, 0.12f, 0);
        glm::mat4 b3 = glm::translate(b, glm::vec3(-0.35f, 0.15f, 0.20f));
        b3 = glm::rotate(b3, glm::radians(-30.0f), glm::vec3(0.2f, 1.0f, 0.0f));
        b3 = glm::scale(b3, glm::vec3(0.55f, 0.40f, 0.60f));
        drawTransformedSphere(shader, sphere, b3, rockGray2, 16.0f, 0.12f, 0);
    }

    // RIGHT CLUSTER (Moved out to X = +15.5 to +18.5)
    // Boulder 4 (Large irregular)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), glm::vec3(16.5f, 0.35f, 1.0f));
        b = glm::rotate(b, glm::radians(-40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 b1 = glm::scale(b, glm::vec3(1.9f, 0.95f, 1.4f));
        drawTransformedSphere(shader, sphere, b1, rockGray2, 16.0f, 0.12f, 0);
        glm::mat4 b2 = glm::translate(b, glm::vec3(-0.30f, 0.45f, 0.20f));
        b2 = glm::rotate(b2, glm::radians(18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b2 = glm::rotate(b2, glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        b2 = glm::scale(b2, glm::vec3(1.2f, 0.50f, 0.90f));
        drawTransformedSphere(shader, sphere, b2, rockGray1, 16.0f, 0.12f, 0);
        glm::mat4 b3 = glm::translate(b, glm::vec3(0.55f, 0.08f, -0.35f));
        b3 = glm::rotate(b3, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b3 = glm::scale(b3, glm::vec3(0.80f, 0.60f, 0.75f));
        drawTransformedSphere(shader, sphere, b3, rockDark, 16.0f, 0.10f, 0);
    }

    // Boulder 5 (Medium rounded)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), glm::vec3(18.0f, 0.28f, 1.8f));
        b = glm::rotate(b, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 b1 = glm::scale(b, glm::vec3(1.2f, 0.70f, 1.0f));
        drawTransformedSphere(shader, sphere, b1, rockLight, 16.0f, 0.10f, 0);
        glm::mat4 b2 = glm::translate(b, glm::vec3(-0.30f, -0.08f, 0.15f));
        b2 = glm::rotate(b2, glm::radians(44.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        b2 = glm::scale(b2, glm::vec3(0.85f, 0.50f, 0.75f));
        drawTransformedSphere(shader, sphere, b2, rockGray2, 16.0f, 0.12f, 0);
    }

    // Boulder 6 (Small pebble)
    {
        glm::mat4 b = glm::translate(glm::mat4(1.0f), glm::vec3(15.2f, 0.15f, 2.2f));
        b = glm::rotate(b, glm::radians(62.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 b1 = glm::scale(b, glm::vec3(0.75f, 0.40f, 0.65f));
        drawTransformedSphere(shader, sphere, b1, rockDark, 16.0f, 0.10f, 0);
    }
}

// Organic standalone rock cluster positioned relative to baseTransform (ideal for showcase)
inline void drawRockCluster(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform)
{
    glm::vec3 rockGray1  = glm::vec3(0.40f, 0.42f, 0.44f);
    glm::vec3 rockGray2  = glm::vec3(0.50f, 0.48f, 0.42f);
    glm::vec3 rockDark   = glm::vec3(0.30f, 0.32f, 0.34f);
    glm::vec3 rockLight  = glm::vec3(0.56f, 0.54f, 0.50f);
    glm::vec3 mossGreen  = glm::vec3(0.32f, 0.40f, 0.28f);

    // Main central boulder
    glm::mat4 b1 = glm::translate(baseTransform, glm::vec3(0.0f, 0.45f, 0.0f));
    b1 = glm::rotate(b1, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    b1 = glm::scale(b1, glm::vec3(2.2f, 1.1f, 1.8f));
    drawTransformedSphere(shader, sphere, b1, rockGray1, 16.0f, 0.12f, 0);

    // Uneven top ridge
    glm::mat4 b2 = glm::translate(baseTransform, glm::vec3(0.35f, 0.85f, -0.20f));
    b2 = glm::rotate(b2, glm::radians(18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    b2 = glm::scale(b2, glm::vec3(1.3f, 0.70f, 1.0f));
    drawTransformedSphere(shader, sphere, b2, rockGray2, 16.0f, 0.12f, 0);

    // Front-left spur
    glm::mat4 b3 = glm::translate(baseTransform, glm::vec3(-0.95f, 0.30f, 0.45f));
    b3 = glm::rotate(b3, glm::radians(-32.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    b3 = glm::scale(b3, glm::vec3(1.1f, 0.65f, 0.95f));
    drawTransformedSphere(shader, sphere, b3, rockDark, 16.0f, 0.10f, 0);

    // Rear-right supporting boulder
    glm::mat4 b4 = glm::translate(baseTransform, glm::vec3(0.85f, 0.35f, -0.45f));
    b4 = glm::rotate(b4, glm::radians(42.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    b4 = glm::scale(b4, glm::vec3(1.2f, 0.68f, 0.85f));
    drawTransformedSphere(shader, sphere, b4, rockLight, 16.0f, 0.10f, 0);

    // Small foreground accent stone
    glm::mat4 b5 = glm::translate(baseTransform, glm::vec3(-0.25f, 0.20f, 0.95f));
    b5 = glm::scale(b5, glm::vec3(0.65f, 0.38f, 0.55f));
    drawTransformedSphere(shader, sphere, b5, rockDark, 16.0f, 0.10f, 0);

    // Moss layer on top
    glm::mat4 bm = glm::translate(baseTransform, glm::vec3(0.05f, 0.96f, 0.15f));
    bm = glm::scale(bm, glm::vec3(0.75f, 0.10f, 0.60f));
    drawTransformedSphere(shader, sphere, bm, mossGreen, 8.0f, 0.05f, 0);
}


// ============================================================================
// OBJECT 10: LIFEGUARD WATCH TOWER
// Elevated wooden observation stilt tower with ladder, platform,
// safety railings, overhanging pitched roof, and hanging lifebuoy.
// ============================================================================
inline void drawLifeguardTower(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform)
{
    glm::vec3 woodWhite   = glm::vec3(0.92f, 0.92f, 0.94f); // Weathered white painted timber
    glm::vec3 woodTeak    = glm::vec3(0.72f, 0.52f, 0.34f); // Teak floorboards & rungs
    glm::vec3 roofRed     = glm::vec3(0.88f, 0.20f, 0.18f); // Lifeguard rescue red
    glm::vec3 lifebuoyRed = glm::vec3(0.90f, 0.15f, 0.12f); // Lifebuoy red
    glm::vec3 lifebuoyWht = glm::vec3(0.96f, 0.96f, 0.96f); // Lifebuoy white

    float legHeight  = 2.6f;  // Height of the stilt platform off the sand
    float baseSpread = 1.25f; // Footprint half-width at ground
    float topSpread  = 1.05f; // Platform half-width

    // 1. FOUR ANGLED STILTS (Legs: wide at sand = 1.25, shrinking at top = 1.05)
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
        {
            float midX = (sx * baseSpread + sx * topSpread) * 0.5f;
            float midZ = (sz * baseSpread + sz * topSpread) * 0.5f;
            glm::mat4 leg = glm::translate(baseTransform, glm::vec3(midX, legHeight * 0.5f, midZ));
            // Inward lean: positive rotation around Z tilts top toward center for sx = +1
            leg = glm::rotate(leg, glm::radians(sx *  4.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            // Inward lean: negative rotation around X tilts top toward center for sz = +1
            leg = glm::rotate(leg, glm::radians(sz * -4.5f), glm::vec3(1.0f, 0.0f, 0.0f));
            leg = glm::scale(leg, glm::vec3(0.10f, legHeight, 0.10f));
            drawTransformedCube(shader, cube, leg, woodWhite);
        }
    }

    // 2. HORIZONTAL PERIMETER SUPPORT BEAMS (Midway up and under platform)
    for (float yFraction : { 0.50f, 0.98f })
    {
        float beamY = legHeight * yFraction;
        float s = baseSpread + (topSpread - baseSpread) * yFraction;

        // Front & Back beams
        for (int sz = -1; sz <= 1; sz += 2)
        {
            glm::mat4 b = glm::translate(baseTransform, glm::vec3(0.0f, beamY, sz * s));
            b = glm::scale(b, glm::vec3(s * 2.05f, 0.08f, 0.08f));
            drawTransformedCube(shader, cube, b, woodWhite);
        }
        // Left & Right beams
        for (int sx = -1; sx <= 1; sx += 2)
        {
            glm::mat4 b = glm::translate(baseTransform, glm::vec3(sx * s, beamY, 0.0f));
            b = glm::scale(b, glm::vec3(0.08f, 0.08f, s * 2.05f));
            drawTransformedCube(shader, cube, b, woodWhite);
        }
    }

    // 3. DIAGONAL CROSS-BRACING (X-Braces on Left, Right, and Back)
    // Mathematically fitted to connect corner-to-corner between the stilts with zero overhang
    float diagLen   = 3.31f;  // Exact diagonal distance between (1.22, 0.08) and (1.02, 2.52)
    float diagAngle = 42.55f; // Exact pitch angle atan2(2.24m, 2.44m)

    // Left and Right side X-braces
    for (int sx = -1; sx <= 1; sx += 2)
    {
        // Diagonal 1: connects bottom-front to top-back
        glm::mat4 d1 = glm::translate(baseTransform, glm::vec3(sx * 1.15f, 1.30f, 0.10f));
        d1 = glm::rotate(d1, glm::radians(sx * 4.5f), glm::vec3(0.0f, 0.0f, 1.0f));
        d1 = glm::rotate(d1, glm::radians(-diagAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        d1 = glm::scale(d1, glm::vec3(0.05f, diagLen, 0.05f));
        drawTransformedCube(shader, cube, d1, woodWhite);

        // Diagonal 2: connects bottom-back to top-front
        glm::mat4 d2 = glm::translate(baseTransform, glm::vec3(sx * 1.15f, 1.30f, -0.10f));
        d2 = glm::rotate(d2, glm::radians(sx * 4.5f), glm::vec3(0.0f, 0.0f, 1.0f));
        d2 = glm::rotate(d2, glm::radians(diagAngle), glm::vec3(1.0f, 0.0f, 0.0f));
        d2 = glm::scale(d2, glm::vec3(0.05f, diagLen, 0.05f));
        drawTransformedCube(shader, cube, d2, woodWhite);
    }

    // Back face X-brace
    // Diagonal 1: connects bottom-left to top-right
    glm::mat4 db1 = glm::translate(baseTransform, glm::vec3(-0.10f, 1.30f, -1.15f));
    db1 = glm::rotate(db1, glm::radians(-4.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    db1 = glm::rotate(db1, glm::radians(-diagAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    db1 = glm::scale(db1, glm::vec3(0.05f, diagLen, 0.05f));
    drawTransformedCube(shader, cube, db1, woodWhite);

    // Diagonal 2: connects bottom-right to top-left
    glm::mat4 db2 = glm::translate(baseTransform, glm::vec3(0.10f, 1.30f, -1.15f));
    db2 = glm::rotate(db2, glm::radians(-4.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    db2 = glm::rotate(db2, glm::radians(diagAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    db2 = glm::scale(db2, glm::vec3(0.05f, diagLen, 0.05f));
    drawTransformedCube(shader, cube, db2, woodWhite);

    // 4. OBSERVATION PLATFORM DECK (Floorboards)
    float deckWidth = topSpread * 2.3f;
    float deckDepth = topSpread * 2.3f;
    glm::mat4 deckMat = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.04f, 0.0f));
    deckMat = glm::scale(deckMat, glm::vec3(deckWidth, 0.08f, deckDepth));
    drawTransformedCube(shader, cube, deckMat, woodTeak);

    // 5. CABIN POSTS (4 upright corner posts)
    float cabinHeight = 1.70f;
    float postOffset = topSpread * 0.95f;
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
        {
            glm::mat4 post = glm::translate(baseTransform, glm::vec3(sx * postOffset, legHeight + 0.08f + cabinHeight * 0.5f, sz * postOffset));
            post = glm::scale(post, glm::vec3(0.08f, cabinHeight, 0.08f));
            drawTransformedCube(shader, cube, post, woodWhite);
        }
    }

    // 6. PERIMETER SAFETY RAILINGS
    float railHeight = 0.85f;
    // Back rail
    glm::mat4 rBack = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + railHeight, -postOffset));
    rBack = glm::scale(rBack, glm::vec3(postOffset * 2.0f, 0.06f, 0.06f));
    drawTransformedCube(shader, cube, rBack, woodWhite);
    // Left rail
    glm::mat4 rLeft = glm::translate(baseTransform, glm::vec3(-postOffset, legHeight + railHeight, 0.0f));
    rLeft = glm::scale(rLeft, glm::vec3(0.06f, 0.06f, postOffset * 2.0f));
    drawTransformedCube(shader, cube, rLeft, woodWhite);
    // Right rail
    glm::mat4 rRight = glm::translate(baseTransform, glm::vec3(postOffset, legHeight + railHeight, 0.0f));
    rRight = glm::scale(rRight, glm::vec3(0.06f, 0.06f, postOffset * 2.0f));
    drawTransformedCube(shader, cube, rRight, woodWhite);
    // Front split rails
    glm::mat4 rFrontL = glm::translate(baseTransform, glm::vec3(-postOffset * 0.65f, legHeight + railHeight, postOffset));
    rFrontL = glm::scale(rFrontL, glm::vec3(postOffset * 0.70f, 0.06f, 0.06f));
    drawTransformedCube(shader, cube, rFrontL, woodWhite);

    glm::mat4 rFrontR = glm::translate(baseTransform, glm::vec3(postOffset * 0.65f, legHeight + railHeight, postOffset));
    rFrontR = glm::scale(rFrontR, glm::vec3(postOffset * 0.70f, 0.06f, 0.06f));
    drawTransformedCube(shader, cube, rFrontR, woodWhite);

    // Baluster slats
    for (int i = 0; i < 5; ++i)
    {
        float t = -postOffset * 0.8f + (float)i * (postOffset * 1.6f / 4.0f);
        glm::mat4 bBal = glm::translate(baseTransform, glm::vec3(t, legHeight + railHeight * 0.5f, -postOffset));
        bBal = glm::scale(bBal, glm::vec3(0.04f, railHeight, 0.04f));
        drawTransformedCube(shader, cube, bBal, woodWhite);

        glm::mat4 lBal = glm::translate(baseTransform, glm::vec3(-postOffset, legHeight + railHeight * 0.5f, t));
        lBal = glm::scale(lBal, glm::vec3(0.04f, railHeight, 0.04f));
        drawTransformedCube(shader, cube, lBal, woodWhite);

        glm::mat4 rBal = glm::translate(baseTransform, glm::vec3(postOffset, legHeight + railHeight * 0.5f, t));
        rBal = glm::scale(rBal, glm::vec3(0.04f, railHeight, 0.04f));
        drawTransformedCube(shader, cube, rBal, woodWhite);
    }

    // 7. PITCHED GABLE ROOF
    float roofBaseY = legHeight + cabinHeight + 0.05f;
    float roofSpan = postOffset * 1.40f;
    float roofTilt = 22.0f;
    // Left roof slope
    glm::mat4 roofLeft = glm::translate(baseTransform, glm::vec3(-roofSpan * 0.45f, roofBaseY + 0.18f, 0.0f));
    roofLeft = glm::rotate(roofLeft, glm::radians(roofTilt), glm::vec3(0.0f, 0.0f, 1.0f));
    roofLeft = glm::scale(roofLeft, glm::vec3(roofSpan, 0.06f, deckDepth * 1.15f));
    drawTransformedCube(shader, cube, roofLeft, roofRed);

    // Right roof slope
    glm::mat4 roofRight = glm::translate(baseTransform, glm::vec3(roofSpan * 0.45f, roofBaseY + 0.18f, 0.0f));
    roofRight = glm::rotate(roofRight, glm::radians(-roofTilt), glm::vec3(0.0f, 0.0f, 1.0f));
    roofRight = glm::scale(roofRight, glm::vec3(roofSpan, 0.06f, deckDepth * 1.15f));
    drawTransformedCube(shader, cube, roofRight, roofRed);

    // Ridge cap
    glm::mat4 ridge = glm::translate(baseTransform, glm::vec3(0.0f, roofBaseY + 0.38f, 0.0f));
    ridge = glm::scale(ridge, glm::vec3(0.12f, 0.08f, deckDepth * 1.16f));
    drawTransformedCube(shader, cube, ridge, woodWhite);

    // 8. ACCESS LADDER (Front entrance - leans forward into the sand)
    float ladderAngle = 18.0f;
    float ladderLen = (legHeight + 0.08f) / cosf(glm::radians(ladderAngle));
    float ladderWidth = 0.55f;
    glm::mat4 ladderPivot = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.06f, postOffset));
    // Negative angle leans forward into the sand away from the tower
    ladderPivot = glm::rotate(ladderPivot, glm::radians(-ladderAngle), glm::vec3(1.0f, 0.0f, 0.0f));

    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 stringer = glm::translate(ladderPivot, glm::vec3(side * ladderWidth * 0.5f, -ladderLen * 0.5f, 0.0f));
        stringer = glm::scale(stringer, glm::vec3(0.05f, ladderLen, 0.05f));
        drawTransformedCube(shader, cube, stringer, woodWhite);
    }
    int rungs = 7;
    for (int i = 1; i <= rungs; ++i)
    {
        float yOffset = -ladderLen * ((float)i / (float)(rungs + 1));
        glm::mat4 rung = glm::translate(ladderPivot, glm::vec3(0.0f, yOffset, 0.0f));
        rung = glm::scale(rung, glm::vec3(ladderWidth, 0.04f, 0.04f));
        drawTransformedCube(shader, cube, rung, woodTeak);
    }

    // 9. LIFEBUOY / RESCUE RING (Hanging on the side railing)
    glm::mat4 buoyCenter = glm::translate(baseTransform, glm::vec3(postOffset + 0.06f, legHeight + railHeight * 0.50f, 0.0f));
    buoyCenter = glm::rotate(buoyCenter, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    int buoySegments = 12;
    for (int i = 0; i < buoySegments; ++i)
    {
        float angle = (float)i * (360.0f / (float)buoySegments);
        glm::mat4 bSeg = glm::rotate(buoyCenter, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        bSeg = glm::translate(bSeg, glm::vec3(0.26f, 0.0f, 0.0f));
        bSeg = glm::scale(bSeg, glm::vec3(0.07f, 0.14f, 0.07f));
        glm::vec3 segCol = (i % 3 == 0) ? lifebuoyRed : lifebuoyWht;
        drawTransformedCube(shader, cube, bSeg, segCol);
    }
}

#endif // SCENE_OBJECTS_H
