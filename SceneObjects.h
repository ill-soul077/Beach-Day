#ifndef SCENE_OBJECTS_H
#define SCENE_OBJECTS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "MeshPrimitives.h"
#include "shaderClass.h"
#include "TextureManager.h"

// ============================================================================
// GROUND PROJECTED SHADOW MATRIX
// Computes planar shadow projection onto sand/water plane Y = groundY.
// Scales, shears, and sweeps shadows dynamically as sun traverses the sky!
// ============================================================================
inline glm::mat4 getGroundShadowMatrix(const glm::vec3& lightPos, float groundY = 0.030f)
{
    glm::vec3 L = glm::normalize(lightPos);
    float ly = glm::max(L.y, 0.08f);
    float sx = glm::clamp(-L.x / ly, -5.5f, 5.5f);
    float sz = glm::clamp(-L.z / ly, -5.5f, 5.5f);

    glm::mat4 s(1.0f);
    s[1][0] = sx;
    s[1][1] = 0.0f;
    s[1][2] = sz;
    s[3][1] = groundY;
    return s;
}

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

inline void drawTransformedFrustum(
    Shader& shader,
    const FrustumMesh& mesh,
    const glm::mat4& model,
    const glm::vec3& color,
    float shininess = 32.0f,
    float specularStrength = 0.25f,
    int unlit = 0)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(glGetUniformLocation(shader.ID, "objectColor"), 1, glm::value_ptr(color));
    glUniform1f(glGetUniformLocation(shader.ID, "specularStrength"), specularStrength);
    glUniform1f(glGetUniformLocation(shader.ID, "shininess"), shininess);
    glUniform1i(glGetUniformLocation(shader.ID, "unlit"), unlit);
    mesh.draw();
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
    const glm::mat4& baseTransform,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true)
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
    if (texMgr && enableTextures)
    {
        texMgr->bind(TEX_SAIL);
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 1);
        glUniform1f(glGetUniformLocation(shader.ID, "texScale"), 1.5f);
    }

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

    if (texMgr && enableTextures)
    {
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 0);
        texMgr->unbind();
    }
}

// ============================================================================
// OBJECT: REALISTIC HIGH-PERFORMANCE MOTORBOAT / SPEEDBOAT
// WITH SEATED ARTICULATED ROBOT DRIVER AT THE HELM
// Featuring:
//  - Deep-V stepped planing hull with chine spray rails and flared bow sheer
//  - Teak-inlaid cockpit floor with driver console and bucket captain's seat
//  - Wraparound tinted aerodynamic windshield with stainless steel frame
//  - Detailed helm station: 4 instrument gauges, glowing GPS screen, throttle lever
//  - Fully steerable 3-spoke sport steering wheel (rotates dynamically with rudder)
//  - Transom-mounted outboard V6 marine engine that pivots dynamically with rudder
//  - Rotating 3-blade hydrofoil bronze propeller
//  - Articulated Robot Pilot seated at the helm, gripping the wheel, head tracking turn
//  - Dynamic stern wash / wake froth trailing behind the propeller
// ============================================================================
inline void drawRealisticMotorboat(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform,
    float rudderAngle = 0.0f,
    float throttle = 0.0f,
    float propellerAngle = 0.0f,
    float speed = 0.0f,
    bool includeRobotDriver = true,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true)
{
    // High-End Marine Color Palette
    glm::vec3 hullNavy       = glm::vec3(0.07f, 0.16f, 0.32f); // Deep oceanic royal navy
    glm::vec3 hullKeelDark   = glm::vec3(0.04f, 0.09f, 0.19f); // Anti-fouling bottom paint
    glm::vec3 hullWhite      = glm::vec3(0.96f, 0.96f, 0.98f); // Glossy marine gelcoat
    glm::vec3 bootStripe     = glm::vec3(0.86f, 0.14f, 0.14f); // Scarlet waterline racing stripe
    glm::vec3 rubRailColor   = glm::vec3(0.14f, 0.15f, 0.18f); // Heavy-duty rubber rub rail
    glm::vec3 teakPlanks     = glm::vec3(0.72f, 0.54f, 0.35f); // Honey marine teak decking
    glm::vec3 teakCaulk      = glm::vec3(0.48f, 0.35f, 0.22f); // Teak margin border
    glm::vec3 chromeRail     = glm::vec3(0.88f, 0.90f, 0.95f); // 316 Polished stainless steel
    glm::vec3 glassTint      = glm::vec3(0.10f, 0.22f, 0.32f); // Dark tinted aerodynamic windshield
    glm::vec3 dashConsole    = glm::vec3(0.18f, 0.20f, 0.24f); // Anti-glare charcoal helm dashboard
    glm::vec3 seatVinyl      = glm::vec3(0.92f, 0.92f, 0.94f); // Marine upholstery vinyl
    glm::vec3 seatTrim       = glm::vec3(0.12f, 0.25f, 0.48f); // Nautical blue seat bolsters
    glm::vec3 dialFace       = glm::vec3(0.94f, 0.94f, 0.90f); // White gauge dials
    glm::vec3 dialNeedle     = glm::vec3(1.00f, 0.20f, 0.10f); // Bright red indicator needles
    glm::vec3 gpsCyan        = glm::vec3(0.00f, 0.88f, 1.00f); // Glowing GPS chartplotter screen
    glm::vec3 engineCowl     = glm::vec3(0.12f, 0.13f, 0.16f); // High-gloss outboard motor cowling
    glm::vec3 engineAccent   = glm::vec3(0.85f, 0.16f, 0.16f); // Engine racing cowling decals
    glm::vec3 propBronze     = glm::vec3(0.82f, 0.62f, 0.28f); // Polished marine bronze propeller
    glm::vec3 wakeFoamColor  = glm::vec3(0.95f, 0.98f, 1.00f); // Aerated churning engine wake foam
    glm::vec3 navRed         = glm::vec3(1.00f, 0.08f, 0.08f); // Port navigation light
    glm::vec3 navGreen       = glm::vec3(0.08f, 0.95f, 0.25f); // Starboard navigation light

    // ------------------------------------------------------------------------
    // 1. DEEP-V PLANING HULL & KEEL (Hydrodynamic V-Deadrise Profile)
    // ------------------------------------------------------------------------
    // Central V-Keel spine (Smooth entry at bow curving up, flat run at stern)
    glm::mat4 keelAft = glm::translate(baseTransform, glm::vec3(0.0f, -0.14f, -0.60f));
    keelAft = glm::scale(keelAft, glm::vec3(0.38f, 0.36f, 2.90f));
    drawTransformedSphere(shader, sphere, keelAft, hullKeelDark, 48.0f, 0.35f);

    glm::mat4 keelBow = glm::translate(baseTransform, glm::vec3(0.0f, 0.02f, 1.30f));
    keelBow = glm::rotate(keelBow, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    keelBow = glm::scale(keelBow, glm::vec3(0.32f, 0.40f, 1.85f));
    drawTransformedSphere(shader, sphere, keelBow, hullKeelDark, 48.0f, 0.35f);

    // Main Lower V-Hull body (Ocean Navy)
    glm::mat4 lowerHullMid = glm::translate(baseTransform, glm::vec3(0.0f, 0.04f, -0.40f));
    lowerHullMid = glm::scale(lowerHullMid, glm::vec3(1.36f, 0.52f, 3.20f));
    drawTransformedSphere(shader, sphere, lowerHullMid, hullNavy, 54.0f, 0.45f);

    glm::mat4 lowerHullBow = glm::translate(baseTransform, glm::vec3(0.0f, 0.12f, 1.15f));
    lowerHullBow = glm::rotate(lowerHullBow, glm::radians(7.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    lowerHullBow = glm::scale(lowerHullBow, glm::vec3(1.18f, 0.56f, 2.10f));
    drawTransformedSphere(shader, sphere, lowerHullBow, hullNavy, 54.0f, 0.45f);

    // Sharp Stem Knife-Edge at Bow
    glm::mat4 bowStem = glm::translate(baseTransform, glm::vec3(0.0f, 0.22f, 2.15f));
    bowStem = glm::rotate(bowStem, glm::radians(32.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    bowStem = glm::scale(bowStem, glm::vec3(0.08f, 0.46f, 0.55f));
    drawTransformedCube(shader, cube, bowStem, hullNavy, 54.0f, 0.45f);

    // Port & Starboard Spray Rails / Chines (Deflects sea spray downwards)
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 chineMat = glm::translate(baseTransform, glm::vec3(side * 0.65f, 0.05f, 0.15f));
        chineMat = glm::scale(chineMat, glm::vec3(0.06f, 0.045f, 3.60f));
        drawTransformedCube(shader, cube, chineMat, hullWhite, 54.0f, 0.50f);
    }

    // Waterline Boot-Stripe (Vivid Scarlet racing accent)
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 stripeMat = glm::translate(baseTransform, glm::vec3(side * 0.685f, 0.16f, 0.08f));
        stripeMat = glm::scale(stripeMat, glm::vec3(0.035f, 0.055f, 3.65f));
        drawTransformedCube(shader, cube, stripeMat, bootStripe, 54.0f, 0.50f);
    }

    // ------------------------------------------------------------------------
    // 2. UPPER TOPSIDES & SHEERLINE (Gleaming White Gelcoat)
    // ------------------------------------------------------------------------
    // Port and Starboard Top Bulwarks / Sheer panels
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 bulwarkMid = glm::translate(baseTransform, glm::vec3(side * 0.68f, 0.32f, -0.45f));
        bulwarkMid = glm::scale(bulwarkMid, glm::vec3(0.09f, 0.28f, 2.95f));
        drawTransformedCube(shader, cube, bulwarkMid, hullWhite, 60.0f, 0.55f);

        // Flared Bow Gunwales (Tapers gently towards the bow stem)
        glm::mat4 bulwarkBow = glm::translate(baseTransform, glm::vec3(side * 0.42f, 0.36f, 1.45f));
        bulwarkBow = glm::rotate(bulwarkBow, glm::radians(-side * 18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bulwarkBow = glm::scale(bulwarkBow, glm::vec3(0.08f, 0.32f, 1.60f));
        drawTransformedCube(shader, cube, bulwarkBow, hullWhite, 60.0f, 0.55f);
    }

    // Flat Transom Stern Board
    glm::mat4 transomBoard = glm::translate(baseTransform, glm::vec3(0.0f, 0.22f, -1.92f));
    transomBoard = glm::scale(transomBoard, glm::vec3(1.36f, 0.44f, 0.10f));
    drawTransformedCube(shader, cube, transomBoard, hullWhite, 60.0f, 0.55f);

    // Motor splashwell cutout recess at center transom
    glm::mat4 splashwellMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.14f, -1.74f));
    splashwellMat = glm::scale(splashwellMat, glm::vec3(0.56f, 0.22f, 0.30f));
    drawTransformedCube(shader, cube, splashwellMat, rubRailColor, 32.0f, 0.20f);

    // Continuous Perimeter Gunwale Rub Rail (Dark rubber bumper)
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 rubRailMid = glm::translate(baseTransform, glm::vec3(side * 0.73f, 0.45f, -0.45f));
        rubRailMid = glm::scale(rubRailMid, glm::vec3(0.06f, 0.055f, 2.95f));
        drawTransformedCube(shader, cube, rubRailMid, rubRailColor, 32.0f, 0.25f);

        glm::mat4 rubRailBow = glm::translate(baseTransform, glm::vec3(side * 0.44f, 0.48f, 1.45f));
        rubRailBow = glm::rotate(rubRailBow, glm::radians(-side * 18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        rubRailBow = glm::scale(rubRailBow, glm::vec3(0.06f, 0.055f, 1.62f));
        drawTransformedCube(shader, cube, rubRailBow, rubRailColor, 32.0f, 0.25f);
    }
    // Bow nose rubber cap
    glm::mat4 noseCap = glm::translate(baseTransform, glm::vec3(0.0f, 0.50f, 2.22f));
    noseCap = glm::scale(noseCap, glm::vec3(0.12f, 0.065f, 0.12f));
    drawTransformedSphere(shader, sphere, noseCap, rubRailColor, 32.0f, 0.25f);

    // Transom rub rail
    glm::mat4 rubRailStern = glm::translate(baseTransform, glm::vec3(0.0f, 0.44f, -1.97f));
    rubRailStern = glm::scale(rubRailStern, glm::vec3(1.42f, 0.055f, 0.06f));
    drawTransformedCube(shader, cube, rubRailStern, rubRailColor, 32.0f, 0.25f);

    // ------------------------------------------------------------------------
    // 3. TRANSOM SWIM PLATFORM & BOARDING LADDER
    // ------------------------------------------------------------------------
    // Teak-inlaid aft swim platform
    glm::mat4 swimPlatform = glm::translate(baseTransform, glm::vec3(0.0f, 0.14f, -2.18f));
    swimPlatform = glm::scale(swimPlatform, glm::vec3(1.24f, 0.055f, 0.45f));
    drawTransformedCube(shader, cube, swimPlatform, teakPlanks, 32.0f, 0.20f);

    // Swim platform perimeter bumper
    glm::mat4 swimRim = glm::translate(baseTransform, glm::vec3(0.0f, 0.14f, -2.41f));
    swimRim = glm::scale(swimRim, glm::vec3(1.26f, 0.065f, 0.045f));
    drawTransformedCube(shader, cube, swimRim, rubRailColor, 32.0f, 0.20f);

    // Stainless steel swim ladder on port side of swim platform
    for (int rung = 0; rung < 3; ++rung)
    {
        glm::mat4 ladderRung = glm::translate(baseTransform, glm::vec3(-0.45f, 0.08f - rung * 0.09f, -2.32f));
        ladderRung = glm::scale(ladderRung, glm::vec3(0.22f, 0.022f, 0.022f));
        drawTransformedCube(shader, cube, ladderRung, chromeRail, 96.0f, 0.85f);
    }

    // ------------------------------------------------------------------------
    // 4. FOREDECK, BOW PULPIT RAILS & MOORING HARDWARE
    // ------------------------------------------------------------------------
    // Closed white foredeck covering bow section
    glm::mat4 foredeckMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.45f, 1.40f));
    foredeckMat = glm::rotate(foredeckMat, glm::radians(3.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    foredeckMat = glm::scale(foredeckMat, glm::vec3(1.10f, 0.045f, 1.55f));
    drawTransformedCube(shader, cube, foredeckMat, hullWhite, 54.0f, 0.50f);

    // Non-skid foredeck center tread runner
    glm::mat4 treadMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.475f, 1.38f));
    treadMat = glm::rotate(treadMat, glm::radians(3.5f), glm::vec3(1.0f, 0.0f, 0.0f));
    treadMat = glm::scale(treadMat, glm::vec3(0.42f, 0.02f, 1.25f));
    drawTransformedCube(shader, cube, treadMat, teakPlanks, 24.0f, 0.15f);

    // Anchor locker hatch with chrome latch
    glm::mat4 anchorHatch = glm::translate(baseTransform, glm::vec3(0.0f, 0.495f, 1.78f));
    anchorHatch = glm::scale(anchorHatch, glm::vec3(0.30f, 0.025f, 0.28f));
    drawTransformedCube(shader, cube, anchorHatch, hullWhite, 48.0f, 0.40f);

    glm::mat4 hatchLatch = glm::translate(baseTransform, glm::vec3(0.0f, 0.515f, 1.66f));
    hatchLatch = glm::scale(hatchLatch, glm::vec3(0.05f, 0.02f, 0.03f));
    drawTransformedCube(shader, cube, hatchLatch, chromeRail, 96.0f, 0.80f);

    // Bow Mooring Cleat (Polished Chrome)
    glm::mat4 bowCleat = glm::translate(baseTransform, glm::vec3(0.0f, 0.52f, 2.12f));
    glm::mat4 cleatBar = glm::scale(bowCleat, glm::vec3(0.18f, 0.025f, 0.035f));
    drawTransformedCube(shader, cube, cleatBar, chromeRail, 96.0f, 0.85f);
    glm::mat4 cleatBase = glm::translate(bowCleat, glm::vec3(0.0f, -0.015f, 0.0f));
    cleatBase = glm::scale(cleatBase, glm::vec3(0.08f, 0.025f, 0.055f));
    drawTransformedCube(shader, cube, cleatBase, chromeRail, 96.0f, 0.85f);

    // Stern Mooring Cleats (Port & Starboard)
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 sternCleat = glm::translate(baseTransform, glm::vec3(side * 0.62f, 0.475f, -1.80f));
        glm::mat4 sternCleatBar = glm::scale(sternCleat, glm::vec3(0.14f, 0.022f, 0.03f));
        drawTransformedCube(shader, cube, sternCleatBar, chromeRail, 96.0f, 0.85f);
    }

    // Stainless Steel Bow Pulpit Railing (Curves around foredeck)
    for (int side = -1; side <= 1; side += 2)
    {
        // Rail upright stanchions
        for (int st = 0; st < 3; ++st)
        {
            float zSt = 0.75f + st * 0.58f;
            float xSt = side * (0.64f - st * 0.13f);
            glm::mat4 stanchion = glm::translate(baseTransform, glm::vec3(xSt, 0.58f, zSt));
            stanchion = glm::scale(stanchion, glm::vec3(0.025f, 0.24f, 0.025f));
            drawTransformedCube(shader, cube, stanchion, chromeRail, 96.0f, 0.85f);
        }

        // Side top rail
        glm::mat4 bowRailSide = glm::translate(baseTransform, glm::vec3(side * 0.48f, 0.70f, 1.42f));
        bowRailSide = glm::rotate(bowRailSide, glm::radians(-side * 14.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bowRailSide = glm::scale(bowRailSide, glm::vec3(0.028f, 0.028f, 1.45f));
        drawTransformedCube(shader, cube, bowRailSide, chromeRail, 96.0f, 0.85f);
    }
    // Front apex joiner rail
    glm::mat4 bowRailApex = glm::translate(baseTransform, glm::vec3(0.0f, 0.71f, 2.16f));
    bowRailApex = glm::scale(bowRailApex, glm::vec3(0.34f, 0.028f, 0.028f));
    drawTransformedCube(shader, cube, bowRailApex, chromeRail, 96.0f, 0.85f);

    // Bi-Color Navigation Light at Bow Tip (Red Port / Green Starboard)
    glm::mat4 navBase = glm::translate(baseTransform, glm::vec3(0.0f, 0.52f, 2.24f));
    navBase = glm::scale(navBase, glm::vec3(0.08f, 0.035f, 0.07f));
    drawTransformedSphere(shader, sphere, navBase, chromeRail, 96.0f, 0.85f);

    glm::mat4 portLens = glm::translate(baseTransform, glm::vec3(-0.025f, 0.535f, 2.25f));
    portLens = glm::scale(portLens, glm::vec3(0.025f, 0.025f, 0.025f));
    drawTransformedSphere(shader, sphere, portLens, navRed, 64.0f, 0.6f, 1);

    glm::mat4 stbdLens = glm::translate(baseTransform, glm::vec3(0.025f, 0.535f, 2.25f));
    stbdLens = glm::scale(stbdLens, glm::vec3(0.025f, 0.025f, 0.025f));
    drawTransformedSphere(shader, sphere, stbdLens, navGreen, 64.0f, 0.6f, 1);

    // ------------------------------------------------------------------------
    // 5. SUNKEN COCKPIT INTERIOR & TEAK FLOORING
    // ------------------------------------------------------------------------
    // Honey Teak Cockpit Sole (Floor planks)
    glm::mat4 cockpitFloor = glm::translate(baseTransform, glm::vec3(0.0f, 0.16f, -0.50f));
    cockpitFloor = glm::scale(cockpitFloor, glm::vec3(1.14f, 0.045f, 2.30f));
    drawTransformedCube(shader, cube, cockpitFloor, teakPlanks, 28.0f, 0.20f);

    // Cockpit perimeter teak margin edging
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 teakMargin = glm::translate(baseTransform, glm::vec3(side * 0.58f, 0.18f, -0.50f));
        teakMargin = glm::scale(teakMargin, glm::vec3(0.04f, 0.03f, 2.30f));
        drawTransformedCube(shader, cube, teakMargin, teakCaulk, 28.0f, 0.20f);
    }

    // Rear Cockpit Passenger Lounge Bench
    glm::mat4 sternBenchBase = glm::translate(baseTransform, glm::vec3(0.0f, 0.26f, -1.48f));
    sternBenchBase = glm::scale(sternBenchBase, glm::vec3(1.10f, 0.18f, 0.42f));
    drawTransformedCube(shader, cube, sternBenchBase, hullWhite, 48.0f, 0.35f);

    glm::mat4 sternBenchCushion = glm::translate(baseTransform, glm::vec3(0.0f, 0.36f, -1.48f));
    sternBenchCushion = glm::scale(sternBenchCushion, glm::vec3(1.08f, 0.07f, 0.40f));
    drawTransformedCube(shader, cube, sternBenchCushion, seatVinyl, 36.0f, 0.30f);

    // Bench Backrest
    glm::mat4 sternBackrest = glm::translate(baseTransform, glm::vec3(0.0f, 0.46f, -1.68f));
    sternBackrest = glm::scale(sternBackrest, glm::vec3(1.08f, 0.18f, 0.08f));
    drawTransformedCube(shader, cube, sternBackrest, seatVinyl, 36.0f, 0.30f);

    // Center accent stripe on lounge bench
    glm::mat4 benchStripe = glm::translate(baseTransform, glm::vec3(0.0f, 0.365f, -1.48f));
    benchStripe = glm::scale(benchStripe, glm::vec3(0.18f, 0.075f, 0.41f));
    drawTransformedCube(shader, cube, benchStripe, seatTrim, 36.0f, 0.30f);

    // ------------------------------------------------------------------------
    // 6. HELM DASHBOARD CONSOLE & AERODYNAMIC WINDSHIELD
    // ------------------------------------------------------------------------
    // Console Main Housing (Charcoal anti-glare)
    glm::mat4 consoleHousing = glm::translate(baseTransform, glm::vec3(0.0f, 0.42f, 0.50f));
    consoleHousing = glm::scale(consoleHousing, glm::vec3(1.04f, 0.28f, 0.36f));
    drawTransformedCube(shader, cube, consoleHousing, dashConsole, 28.0f, 0.15f);

    // Slanted Instrument Binnacle Panel (Faced 25 deg towards driver)
    glm::mat4 binnacleMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.52f, 0.45f));
    binnacleMat = glm::rotate(binnacleMat, glm::radians(-26.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 binnaclePlate = glm::scale(binnacleMat, glm::vec3(0.96f, 0.20f, 0.035f));
    drawTransformedCube(shader, cube, binnaclePlate, dashConsole, 28.0f, 0.15f);

    // Center 7-inch Glowing GPS Chartplotter Display
    glm::mat4 gpsMat = glm::translate(binnacleMat, glm::vec3(0.0f, 0.02f, 0.022f));
    gpsMat = glm::scale(gpsMat, glm::vec3(0.26f, 0.13f, 0.015f));
    drawTransformedCube(shader, cube, gpsMat, gpsCyan, 64.0f, 0.6f, 1);

    // Circular Marine Instrument Gauges (Speedometer, Tachometer, Trim, Fuel)
    const float gaugePositions[4] = { -0.36f, -0.21f, 0.21f, 0.36f };
    for (int g = 0; g < 4; ++g)
    {
        // Chrome Bezel
        glm::mat4 gaugeBezel = glm::translate(binnacleMat, glm::vec3(gaugePositions[g], 0.02f, 0.020f));
        gaugeBezel = glm::scale(gaugeBezel, glm::vec3(0.09f, 0.09f, 0.015f));
        drawTransformedSphere(shader, sphere, gaugeBezel, chromeRail, 96.0f, 0.85f);

        // White dial face
        glm::mat4 gaugeFace = glm::translate(binnacleMat, glm::vec3(gaugePositions[g], 0.02f, 0.026f));
        gaugeFace = glm::scale(gaugeFace, glm::vec3(0.075f, 0.075f, 0.010f));
        drawTransformedSphere(shader, sphere, gaugeFace, dialFace, 48.0f, 0.40f);

        // Red indicator needle
        float needleAngle = 30.0f + (g * 45.0f) + (g == 0 ? glm::clamp(speed * 8.0f, 0.0f, 180.0f) : 0.0f);
        glm::mat4 needleMat = glm::translate(binnacleMat, glm::vec3(gaugePositions[g], 0.02f, 0.032f));
        needleMat = glm::rotate(needleMat, glm::radians(needleAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        needleMat = glm::scale(needleMat, glm::vec3(0.012f, 0.055f, 0.008f));
        drawTransformedCube(shader, cube, needleMat, dialNeedle, 48.0f, 0.50f);
    }

    // Chrome Throttle & Shift Binnacle Lever (Right side of console)
    glm::mat4 throttleBox = glm::translate(baseTransform, glm::vec3(0.38f, 0.48f, 0.35f));
    throttleBox = glm::scale(throttleBox, glm::vec3(0.08f, 0.09f, 0.16f));
    drawTransformedCube(shader, cube, throttleBox, dashConsole, 48.0f, 0.30f);

    // Throttle handle that tilts with throttle input
    glm::mat4 throttleLever = glm::translate(baseTransform, glm::vec3(0.38f, 0.53f, 0.35f));
    throttleLever = glm::rotate(throttleLever, glm::radians(-throttle * 32.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 leverShaft = glm::scale(throttleLever, glm::vec3(0.022f, 0.14f, 0.022f));
    drawTransformedCube(shader, cube, leverShaft, chromeRail, 96.0f, 0.85f);
    glm::mat4 leverKnob = glm::translate(throttleLever, glm::vec3(0.0f, 0.08f, 0.0f));
    leverKnob = glm::scale(leverKnob, glm::vec3(0.045f, 0.045f, 0.045f));
    drawTransformedSphere(shader, sphere, leverKnob, engineAccent, 96.0f, 0.85f);

    // Wraparound Tinted Windshield
    // Front Center Windshield Panel (swept back at 35 degrees)
    glm::mat4 frontWindshield = glm::translate(baseTransform, glm::vec3(0.0f, 0.64f, 0.58f));
    frontWindshield = glm::rotate(frontWindshield, glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 frontGlass = glm::scale(frontWindshield, glm::vec3(1.06f, 0.28f, 0.035f));
    drawTransformedCube(shader, cube, frontGlass, glassTint, 96.0f, 0.80f);

    // Windshield Top Chrome Frame
    glm::mat4 windshieldFrameTop = glm::translate(frontWindshield, glm::vec3(0.0f, 0.14f, 0.0f));
    windshieldFrameTop = glm::scale(windshieldFrameTop, glm::vec3(1.08f, 0.025f, 0.04f));
    drawTransformedCube(shader, cube, windshieldFrameTop, chromeRail, 96.0f, 0.85f);

    // Port & Starboard Wing Windshield Panels
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 sideWindshield = glm::translate(baseTransform, glm::vec3(side * 0.58f, 0.58f, 0.40f));
        sideWindshield = glm::rotate(sideWindshield, glm::radians(-side * 28.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        sideWindshield = glm::rotate(sideWindshield, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        sideWindshield = glm::scale(sideWindshield, glm::vec3(0.03f, 0.20f, 0.36f));
        drawTransformedCube(shader, cube, sideWindshield, glassTint, 96.0f, 0.80f);
    }

    // ------------------------------------------------------------------------
    // 7. SPORT STEERING WHEEL (Dynamically Rotates with Rudder Angle!)
    // ------------------------------------------------------------------------
    // Steering column angled towards driver
    glm::mat4 steerCol = glm::translate(baseTransform, glm::vec3(0.0f, 0.48f, 0.36f));
    steerCol = glm::rotate(steerCol, glm::radians(-26.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 colShaft = glm::scale(steerCol, glm::vec3(0.045f, 0.045f, 0.12f));
    drawTransformedCube(shader, cube, colShaft, dashConsole, 48.0f, 0.30f);

    // Steering wheel plane: Rotates around steering axle with rudderAngle
    glm::mat4 steerWheelCenter = glm::translate(steerCol, glm::vec3(0.0f, 0.0f, -0.07f));
    steerWheelCenter = glm::rotate(steerWheelCenter, glm::radians(rudderAngle * 2.8f), glm::vec3(0.0f, 0.0f, 1.0f));

    // Center Hub with polished chrome cap
    glm::mat4 hubCap = glm::scale(steerWheelCenter, glm::vec3(0.075f, 0.075f, 0.035f));
    drawTransformedSphere(shader, sphere, hubCap, chromeRail, 96.0f, 0.90f);

    // 3 Chrome Spokes (at 0, 120, 240 degrees)
    for (int sp = 0; sp < 3; ++sp)
    {
        glm::mat4 spoke = glm::rotate(steerWheelCenter, glm::radians(sp * 120.0f + 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        spoke = glm::translate(spoke, glm::vec3(0.0f, 0.08f, 0.0f));
        spoke = glm::scale(spoke, glm::vec3(0.022f, 0.14f, 0.018f));
        drawTransformedCube(shader, cube, spoke, chromeRail, 96.0f, 0.90f);
    }

    // Outer circular sport wheel rim (approximated with smooth segmented ring)
    const int rimSegs = 12;
    for (int r = 0; r < rimSegs; ++r)
    {
        float theta = (float)r * (360.0f / (float)rimSegs);
        glm::mat4 rimSeg = glm::rotate(steerWheelCenter, glm::radians(theta), glm::vec3(0.0f, 0.0f, 1.0f));
        rimSeg = glm::translate(rimSeg, glm::vec3(0.0f, 0.155f, 0.0f));
        rimSeg = glm::scale(rimSeg, glm::vec3(0.082f, 0.030f, 0.028f));
        drawTransformedCube(shader, cube, rimSeg, rubRailColor, 48.0f, 0.35f);
    }

    // ------------------------------------------------------------------------
    // 8. CAPTAIN'S BUCKET SEAT (Ergonomic Helm Chair)
    // ------------------------------------------------------------------------
    // Stainless steel chair pedestal
    glm::mat4 seatPedestal = glm::translate(baseTransform, glm::vec3(0.0f, 0.22f, 0.02f));
    seatPedestal = glm::scale(seatPedestal, glm::vec3(0.085f, 0.12f, 0.085f));
    drawTransformedCube(shader, cube, seatPedestal, chromeRail, 96.0f, 0.85f);

    // Contoured seat cushion base
    glm::mat4 seatCushion = glm::translate(baseTransform, glm::vec3(0.0f, 0.29f, 0.02f));
    glm::mat4 cushionBase = glm::scale(seatCushion, glm::vec3(0.56f, 0.075f, 0.48f));
    drawTransformedCube(shader, cube, cushionBase, seatVinyl, 40.0f, 0.35f);

    // Side bolster supports
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 bolster = glm::translate(baseTransform, glm::vec3(side * 0.25f, 0.34f, 0.02f));
        bolster = glm::scale(bolster, glm::vec3(0.08f, 0.11f, 0.44f));
        drawTransformedCube(shader, cube, bolster, seatTrim, 40.0f, 0.35f);
    }

    // High-back contoured backrest
    glm::mat4 seatBackrest = glm::translate(baseTransform, glm::vec3(0.0f, 0.50f, -0.22f));
    seatBackrest = glm::rotate(seatBackrest, glm::radians(6.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 backrestPad = glm::scale(seatBackrest, glm::vec3(0.52f, 0.36f, 0.08f));
    drawTransformedCube(shader, cube, backrestPad, seatVinyl, 40.0f, 0.35f);

    // Blue racing center stripe on backrest
    glm::mat4 backrestStripe = glm::translate(seatBackrest, glm::vec3(0.0f, 0.0f, 0.01f));
    backrestStripe = glm::scale(backrestStripe, glm::vec3(0.14f, 0.365f, 0.082f));
    drawTransformedCube(shader, cube, backrestStripe, seatTrim, 40.0f, 0.35f);

    // ------------------------------------------------------------------------
    // 9. ARTICULATED ROBOT DRIVER SEATED AT THE HELM
    // Scaled to 0.8x to maintain proper proportions relative to the boat
    // (A 20ft motorboat cockpit seats a human-sized occupant proportionally)
    // ------------------------------------------------------------------------
    if (includeRobotDriver)
    {
        // Place robot at seat position, scaled down for proper boat proportions
        glm::mat4 driverBase = glm::translate(baseTransform, glm::vec3(0.0f, 0.32f, 0.02f));
        driverBase = glm::scale(driverBase, glm::vec3(0.80f, 0.80f, 0.80f)); // Proportional sizing

        // Head turns smoothly towards the turn direction (scans ahead into the curve!)
        float driverHeadYaw = -rudderAngle * 0.85f;
        float driverHeadPitch = -5.0f; // Looking slightly down at instruments

        // Dynamic Arm Angles: Hands gripping the turning steering wheel at 10 & 2 o'clock
        float driverLeftArm  = -48.0f + rudderAngle * 0.45f;
        float driverRightArm = -48.0f - rudderAngle * 0.45f;

        // Seated leg pose: 85 deg bend so feet rest firmly on teak floor
        float driverLegBend  = 85.0f;

        drawRobot(
            shader,
            cube,
            sphere,
            driverBase,
            driverHeadYaw,
            driverHeadPitch,
            driverLeftArm,
            driverRightArm,
            driverLegBend
        );
    }

    // ------------------------------------------------------------------------
    // 10. TRANSOM-MOUNTED STEERABLE OUTBOARD V6 ENGINE
    // (Swivels dynamically with rudderAngle, trims up with forward speed!)
    // ------------------------------------------------------------------------
    // Transom mounting bracket
    glm::mat4 motorBracket = glm::translate(baseTransform, glm::vec3(0.0f, 0.22f, -1.98f));
    motorBracket = glm::scale(motorBracket, glm::vec3(0.32f, 0.22f, 0.12f));
    drawTransformedCube(shader, cube, motorBracket, chromeRail, 96.0f, 0.85f);

    // Dynamic Swivel Pivot (Steering Yaw + Hydrodynamic Trim Pitch)
    glm::mat4 enginePivot = glm::translate(baseTransform, glm::vec3(0.0f, 0.18f, -2.10f));
    enginePivot = glm::rotate(enginePivot, glm::radians(rudderAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    float trimAngle = (speed > 1.0f) ? glm::clamp(speed * 0.45f, 0.0f, 8.0f) : 0.0f;
    enginePivot = glm::rotate(enginePivot, glm::radians(trimAngle), glm::vec3(1.0f, 0.0f, 0.0f));

    // Powerhead Engine Cowling (Glossy Black Top Cover)
    glm::mat4 cowlMat = glm::translate(enginePivot, glm::vec3(0.0f, 0.28f, -0.06f));
    cowlMat = glm::scale(cowlMat, glm::vec3(0.38f, 0.42f, 0.62f));
    drawTransformedSphere(shader, sphere, cowlMat, engineCowl, 72.0f, 0.65f);

    // Engine top air intake scoop
    glm::mat4 airScoop = glm::translate(enginePivot, glm::vec3(0.0f, 0.48f, -0.02f));
    airScoop = glm::scale(airScoop, glm::vec3(0.18f, 0.06f, 0.32f));
    drawTransformedCube(shader, cube, airScoop, rubRailColor, 48.0f, 0.35f);

    // Engine side red racing stripe
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 decalMat = glm::translate(enginePivot, glm::vec3(side * 0.195f, 0.28f, -0.06f));
        decalMat = glm::scale(decalMat, glm::vec3(0.02f, 0.08f, 0.48f));
        drawTransformedCube(shader, cube, decalMat, engineAccent, 72.0f, 0.70f);
    }

    // Midsection Drive Shaft Leg (Exhaust housing extending down into water)
    glm::mat4 midLeg = glm::translate(enginePivot, glm::vec3(0.0f, -0.06f, -0.08f));
    midLeg = glm::scale(midLeg, glm::vec3(0.18f, 0.36f, 0.30f));
    drawTransformedCube(shader, cube, midLeg, engineCowl, 54.0f, 0.45f);

    // Lower Gearcase Torpedo (Bullet housing)
    glm::mat4 torpedo = glm::translate(enginePivot, glm::vec3(0.0f, -0.28f, -0.10f));
    torpedo = glm::scale(torpedo, glm::vec3(0.15f, 0.15f, 0.50f));
    drawTransformedSphere(shader, sphere, torpedo, engineCowl, 64.0f, 0.55f);

    // Horizontal Anti-Ventilation / Cavitation Plate
    glm::mat4 cavPlate = glm::translate(enginePivot, glm::vec3(0.0f, -0.21f, -0.12f));
    cavPlate = glm::scale(cavPlate, glm::vec3(0.30f, 0.025f, 0.38f));
    drawTransformedCube(shader, cube, cavPlate, engineCowl, 64.0f, 0.55f);

    // Hydrodynamic Skeg Fin (Vertical bottom steering skeg)
    glm::mat4 skegFin = glm::translate(enginePivot, glm::vec3(0.0f, -0.42f, -0.16f));
    skegFin = glm::rotate(skegFin, glm::radians(12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    skegFin = glm::scale(skegFin, glm::vec3(0.035f, 0.18f, 0.26f));
    drawTransformedCube(shader, cube, skegFin, engineCowl, 64.0f, 0.55f);

    // 3-Blade Polished Marine Bronze Propeller (Continuously spins around prop shaft)
    glm::mat4 propShaftMat = glm::translate(enginePivot, glm::vec3(0.0f, -0.28f, -0.36f));
    propShaftMat = glm::rotate(propShaftMat, glm::radians(propellerAngle), glm::vec3(0.0f, 0.0f, 1.0f));

    // Propeller Center Bullet Hub
    glm::mat4 propHub = glm::scale(propShaftMat, glm::vec3(0.09f, 0.09f, 0.14f));
    drawTransformedSphere(shader, sphere, propHub, propBronze, 96.0f, 0.90f);

    // 3 Hydrofoil Bronze Propeller Blades
    for (int b = 0; b < 3; ++b)
    {
        glm::mat4 bladeMat = glm::rotate(propShaftMat, glm::radians(b * 120.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        bladeMat = glm::rotate(bladeMat, glm::radians(24.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        bladeMat = glm::translate(bladeMat, glm::vec3(0.0f, 0.11f, 0.0f));
        bladeMat = glm::scale(bladeMat, glm::vec3(0.055f, 0.15f, 0.016f));
        drawTransformedCube(shader, cube, bladeMat, propBronze, 96.0f, 0.90f);
    }

    // ------------------------------------------------------------------------
    // 11. DYNAMIC PROPELLER WASH, STERN WAKE, & BOW SPRAY
    // Speed-dependent wake system with realistic V-pattern and spray
    // ------------------------------------------------------------------------
    float absSpd = fabs(speed);
    if (absSpd > 0.15f || fabs(throttle) > 0.05f)
    {
        float speedNorm = glm::clamp(absSpd / 12.0f, 0.0f, 1.0f);

        // --- Propeller Wash (churning water directly behind prop) ---
        glm::mat4 propFroth = glm::translate(enginePivot, glm::vec3(0.0f, -0.26f, -0.55f));
        float frothScale = 0.18f + speedNorm * 0.28f;
        propFroth = glm::scale(propFroth, glm::vec3(frothScale, frothScale * 0.6f, frothScale * 2.0f));
        drawTransformedSphere(shader, sphere, propFroth, wakeFoamColor, 20.0f, 0.18f, 1);

        // --- Rooster Tail Spray (high-speed water jet flung upward) ---
        if (absSpd > 3.0f)
        {
            float roosterHeight = speedNorm * 0.35f;
            glm::mat4 roosterTail = glm::translate(enginePivot, glm::vec3(0.0f, -0.10f + roosterHeight, -0.75f));
            roosterTail = glm::scale(roosterTail, glm::vec3(0.15f + speedNorm * 0.1f, roosterHeight * 1.2f, 0.25f));
            drawTransformedSphere(shader, sphere, roosterTail, wakeFoamColor, 16.0f, 0.12f, 1);
        }

        // --- Expanding V-Shaped Kelvin Wake Trail ---
        // Number of wake segments scales with speed (more segments = longer trail)
        int numWakeSegs = 3 + (int)(speedNorm * 5.0f);
        for (int w = 1; w <= numWakeSegs; ++w)
        {
            float aftDist = -2.3f - w * 0.95f;
            // Kelvin angle: arms spread at ~19.5 degrees
            float spreadWidth = 0.35f + w * 0.33f * (0.7f + speedNorm * 0.3f);
            float wakeSize = 0.20f + w * 0.08f;
            // Fade out with distance
            float fadeFactor = glm::clamp(1.0f - (float)w / (float)(numWakeSegs + 1), 0.1f, 1.0f);

            for (int side = -1; side <= 1; side += 2)
            {
                glm::mat4 wakePuff = glm::translate(baseTransform, glm::vec3(side * spreadWidth, -0.04f, aftDist));
                wakePuff = glm::scale(wakePuff, glm::vec3(wakeSize * 1.0f, 0.05f * fadeFactor, wakeSize * 0.7f));
                drawTransformedSphere(shader, sphere, wakePuff, wakeFoamColor, 24.0f, 0.08f * fadeFactor);
            }

            // Centerline turbulent wash between the V-arms
            if (w <= numWakeSegs / 2)
            {
                glm::mat4 centerWash = glm::translate(baseTransform, glm::vec3(0.0f, -0.06f, aftDist + 0.4f));
                centerWash = glm::scale(centerWash, glm::vec3(spreadWidth * 0.4f, 0.03f, 0.35f));
                drawTransformedSphere(shader, sphere, centerWash, wakeFoamColor, 18.0f, 0.06f * fadeFactor);
            }
        }

        // --- Bow Spray (chine spray kicked up at speed) ---
        if (absSpd > 2.0f)
        {
            for (int side = -1; side <= 1; side += 2)
            {
                float sprayHeight = speedNorm * 0.12f;
                glm::mat4 bowSpray = glm::translate(baseTransform, glm::vec3(side * 0.72f, 0.08f + sprayHeight, 1.5f));
                float spraySize = 0.10f + speedNorm * 0.15f;
                bowSpray = glm::scale(bowSpray, glm::vec3(spraySize, spraySize * 0.5f, spraySize * 1.6f));
                drawTransformedSphere(shader, sphere, bowSpray, wakeFoamColor, 16.0f, 0.10f);
            }
        }
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
    float trunkCurveAngle = 3.2f,
    float windSway = 0.0f,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true)
{
    glm::vec3 barkColor    = glm::vec3(0.48f, 0.32f, 0.18f); // Palm bark brown
    glm::vec3 ringColor    = glm::vec3(0.38f, 0.24f, 0.12f); // Segment ring groove
    glm::vec3 coconutColor = glm::vec3(0.30f, 0.17f, 0.08f); // Coconuts
    glm::vec3 stemColor    = glm::vec3(0.28f, 0.52f, 0.16f); // Frond stem
    glm::vec3 leafGreen1   = glm::vec3(0.12f, 0.60f, 0.16f); // Lush tropical green
    glm::vec3 leafGreen2   = glm::vec3(0.15f, 0.68f, 0.20f); // Bright highlight green

    // 1. CURVED SEGMENTED TRUNK (Stacked and progressively tilted nodes with Wind Sway)
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

        // Advance to next segment with slight tilt + dynamic wind sway for natural curve
        currentTransform = glm::translate(currentTransform, glm::vec3(0.0f, segHeight, 0.0f));
        currentTransform = glm::rotate(currentTransform, glm::radians(trunkCurveAngle + windSway * 0.35f), glm::vec3(0.0f, 0.0f, 1.0f));
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
            // Pitch downward from crown + wind sway
            spineNode = glm::rotate(spineNode, glm::radians(tier.initialPitch + windSway * 0.8f), glm::vec3(1.0f, 0.0f, 0.0f));

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

                // Apply progressive droop to curve the spine downward + animated wind sway
                float segSway = windSway * 1.5f * (float)(seg + 1) / 6.0f;
                spineNode = glm::rotate(spineNode, glm::radians(droopAngles[seg] + segSway), glm::vec3(1.0f, 0.0f, 0.0f));

                // 3a. Central Frond Rachis / Stem
                glm::mat4 stemMat = glm::translate(spineNode, glm::vec3(0.0f, 0.0f, segLen * 0.5f));
                stemMat = glm::scale(stemMat, glm::vec3(0.045f * segTaper, 0.030f * segTaper, segLen));
                drawTransformedCube(shader, cube, stemMat, stemColor);

                // 3b. Pairs of Angled Feathery Leaflets (Left & Right)
                if (seg >= 1) // skip bare stem base near crown
                {
                    if (texMgr && enableTextures)
                    {
                        texMgr->bind(TEX_PALM);
                        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 1);
                        glUniform1f(glGetUniformLocation(shader.ID, "texScale"), 1.0f);
                    }

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

                    if (texMgr && enableTextures)
                    {
                        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 0);
                        texMgr->unbind();
                    }
                }

                // Advance to the end of this segment
                spineNode = glm::translate(spineNode, glm::vec3(0.0f, 0.0f, segLen));
            }
        }
    }
}

// Short banana plant with a layered pseudostem and broad wind-tossed leaves.
inline void drawBananaPlant(
    Shader& shader,
    const CubeMesh& cube,
    const glm::mat4& baseTransform,
    float windSway = 0.0f)
{
    glm::vec3 stemGreen(0.38f, 0.48f, 0.12f);
    glm::vec3 leafLight(0.24f, 0.62f, 0.16f);
    glm::vec3 leafDark(0.10f, 0.42f, 0.10f);

    glm::mat4 stem = glm::translate(baseTransform, glm::vec3(0.0f, 1.15f, 0.0f));
    stem = glm::scale(stem, glm::vec3(0.34f, 2.30f, 0.34f));
    drawTransformedCube(shader, cube, stem, stemGreen, 9.0f, 0.06f);

    for (int i = 0; i < 9; ++i)
    {
        float yaw = (float)i * 40.0f + (float)(i % 2) * 9.0f;
        float droop = 8.0f + (float)(i % 3) * 8.0f + windSway * 0.55f;
        glm::mat4 leaf = glm::translate(baseTransform, glm::vec3(0.0f, 2.30f, 0.0f));
        leaf = glm::rotate(leaf, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        leaf = glm::rotate(leaf, glm::radians(droop), glm::vec3(1.0f, 0.0f, 0.0f));
        leaf = glm::translate(leaf, glm::vec3(0.0f, 0.0f, 0.88f));
        leaf = glm::scale(leaf, glm::vec3(0.48f, 0.045f, 1.78f));
        drawTransformedCube(shader, cube, leaf, (i % 2 == 0) ? leafLight : leafDark, 8.0f, 0.05f);
    }
}

// Pandanus / screw-pine: multiple leaning stems and a dense crown of narrow,
// spiky leaves. Its silhouette is deliberately distinct from coconut palms.
inline void drawPandanusTree(
    Shader& shader,
    const CubeMesh& cube,
    const glm::mat4& baseTransform,
    float windSway = 0.0f)
{
    glm::vec3 bark(0.30f, 0.23f, 0.12f);
    glm::vec3 leafA(0.08f, 0.36f, 0.10f);
    glm::vec3 leafB(0.16f, 0.52f, 0.13f);

    for (int stemIndex = -1; stemIndex <= 1; ++stemIndex)
    {
        glm::mat4 stem = glm::translate(baseTransform, glm::vec3((float)stemIndex * 0.24f, 0.95f, 0.0f));
        stem = glm::rotate(stem, glm::radians((float)stemIndex * 11.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        stem = glm::scale(stem, glm::vec3(0.20f, 1.90f, 0.20f));
        drawTransformedCube(shader, cube, stem, bark, 8.0f, 0.05f);
    }

    for (int i = 0; i < 14; ++i)
    {
        float yaw = (float)i * (360.0f / 14.0f);
        float pitch = -8.0f + (float)(i % 4) * 9.0f + windSway * 0.45f;
        glm::mat4 leaf = glm::translate(baseTransform, glm::vec3(0.0f, 1.92f, 0.0f));
        leaf = glm::rotate(leaf, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        leaf = glm::rotate(leaf, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        leaf = glm::translate(leaf, glm::vec3(0.0f, 0.0f, 0.78f));
        leaf = glm::scale(leaf, glm::vec3(0.16f, 0.038f, 1.58f));
        drawTransformedCube(shader, cube, leaf, (i % 3 == 0) ? leafB : leafA, 7.0f, 0.04f);
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
    const glm::vec3& seaCenter = glm::vec3(0.0f, -0.05f, 160.0f),
    float width = 500.0f,
    float depth = 320.0f,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true)
{
    glm::vec3 waterColor = glm::vec3(0.08f, 0.48f, 0.65f); // Tropical coastal ocean blue

    if (texMgr && enableTextures)
    {
        texMgr->bind(TEX_WATER);
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 1);
        glUniform1f(glGetUniformLocation(shader.ID, "texScale"), 36.0f);
    }
    // GOURAUD SHADING for expansive sea water surface
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 1);

    glm::mat4 seaMat = glm::translate(glm::mat4(1.0f), seaCenter);
    seaMat = glm::scale(seaMat, glm::vec3(width, 0.2f, depth));

    // Water has high shininess and specular highlight for sun reflection
    drawTransformedCube(shader, cube, seaMat, waterColor, 128.0f, 0.85f, 0);

    if (texMgr && enableTextures)
    {
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 0);
        texMgr->unbind();
    }

    // Restore to default Phong
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 0);
}

// ============================================================================
// GERSTNER OCEAN WAVE SIMULATION (C++ CPU EVALUATION FOR BUOYANCY)
// Evaluates the same six-component gravity-wave spectrum as water.vert for buoyancy.
// Returns displaced world position and surface normal.
// ============================================================================
inline void getGerstnerWaveAt(
    float x, float z, float time,
    glm::vec3& outPos, glm::vec3& outNormal)
{
    struct WaveDef {
        glm::vec2 dir;
        float wavelength;
        float amplitude;
        float steepness;
        float phaseOffset;
    };

    const WaveDef waves[6] = {
        { glm::normalize(glm::vec2(-0.06f, -0.998f)), 46.0f, 0.40f, 0.30f, 0.0f },
        { glm::normalize(glm::vec2( 0.05f, -0.999f)), 31.0f, 0.23f, 0.27f, 1.7f },
        { glm::normalize(glm::vec2(-0.13f, -0.991f)), 21.0f, 0.13f, 0.22f, 3.8f },
        { glm::normalize(glm::vec2( 0.11f, -0.994f)), 13.0f, 0.07f, 0.17f, 5.1f },
        { glm::normalize(glm::vec2(-0.18f, -0.984f)),  8.5f, 0.04f, 0.12f, 2.6f },
        { glm::normalize(glm::vec2( 0.17f, -0.985f)),  5.8f, 0.02f, 0.08f, 4.4f }
    };

    float outsideDistance = glm::max(-getIslandSignedDistance(x, z), 0.0f);
    float shoreTaper = glm::smoothstep(0.35f, 7.0f, outsideDistance);
    float shoaling = 1.0f + 0.22f
        * (1.0f - glm::smoothstep(15.0f, 38.0f, outsideDistance))
        * glm::smoothstep(2.0f, 9.0f, outsideDistance);
    glm::vec2 towardIslandVector(-x / (110.0f * 110.0f),
                                 -(z + 100.0f) / (155.0f * 155.0f));
    glm::vec2 towardIsland = glm::length(towardIslandVector) > 0.00001f
        ? glm::normalize(towardIslandVector) : glm::vec2(0.0f, -1.0f);
    float refraction = (1.0f - glm::smoothstep(10.0f, 48.0f, outsideDistance)) * 0.58f;

    glm::vec3 displacement(0.0f);
    glm::vec3 normalAccum(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < 6; ++i)
    {
        float k = 2.0f * 3.14159265f / waves[i].wavelength;
        float omega = sqrtf(9.81f * k);
        glm::vec2 refractedDirection = glm::normalize(glm::mix(waves[i].dir, towardIsland, refraction));
        float f = k * glm::dot(refractedDirection, glm::vec2(x, z)) - omega * time + waves[i].phaseOffset;

        float cosf_val = cosf(f);
        float sinf_val = sinf(f);

        float amp = waves[i].amplitude * shoreTaper * shoaling;
        float q   = waves[i].steepness;

        displacement.x += q * amp * refractedDirection.x * cosf_val;
        displacement.y += amp * sinf_val;
        displacement.z += q * amp * refractedDirection.y * cosf_val;

        normalAccum.x -= refractedDirection.x * (k * amp) * cosf_val;
        normalAccum.y -= q * (k * amp) * sinf_val;
        normalAccum.z -= refractedDirection.y * (k * amp) * cosf_val;
    }

    outPos = glm::vec3(x + displacement.x, displacement.y, z + displacement.z);
    outNormal = glm::normalize(normalAccum);
}

// ============================================================================
// OBJECT 7B: REALISTIC GERSTNER OCEAN WATER SURFACE (Diagrams A & B)
// Animated GPU-displaced Gerstner waves approaching shore with crest foam
// Now with dynamic boat wake/bow wave interaction
// ============================================================================
inline void drawOceanWaves(
    Shader& waterShader,
    const WaterGridMesh& waterMesh,
    const glm::mat4& view,
    const glm::mat4& proj,
    const glm::vec3& camPos,
    const glm::vec3& lightDir,
    const glm::vec3& lightColor,
    float ambientStrength,
    float time,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true,
    const glm::vec3& boatPos = glm::vec3(0.0f),
    float boatHeading = 0.0f,
    float boatSpeed = 0.0f)
{
    waterShader.Activate();

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(waterShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(waterShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(waterShader.ID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(glGetUniformLocation(waterShader.ID, "viewPos"), 1, glm::value_ptr(camPos));
    glUniform3fv(glGetUniformLocation(waterShader.ID, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(waterShader.ID, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform1f(glGetUniformLocation(waterShader.ID, "ambientStrength"), ambientStrength);
    glUniform1f(glGetUniformLocation(waterShader.ID, "time"), time);

    // Boat interaction uniforms for dynamic wake and bow wave
    glUniform3fv(glGetUniformLocation(waterShader.ID, "boatPos"), 1, glm::value_ptr(boatPos));
    glUniform1f(glGetUniformLocation(waterShader.ID, "boatHeading"), boatHeading);
    glUniform1f(glGetUniformLocation(waterShader.ID, "boatSpeed"), boatSpeed);

    if (texMgr && enableTextures)
    {
        texMgr->bind(TEX_FOAM, GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(waterShader.ID, "foamTex"), 0);
        glUniform1i(glGetUniformLocation(waterShader.ID, "useFoamTex"), 1);
    }
    else
    {
        glUniform1i(glGetUniformLocation(waterShader.ID, "useFoamTex"), 0);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    waterMesh.draw();

    glDisable(GL_BLEND);

    if (texMgr && enableTextures)
    {
        texMgr->unbind(GL_TEXTURE0);
    }
}

// ============================================================================
// OBJECT 7C: COASTAL BATHYMETRY & SUBMERGED SEABED (Diagram B)
// Sloping seabed from beach berm down to deep ocean with sandbar & trough
// ============================================================================
inline void drawCoastalBathymetryAndSeabed(
    Shader& shader,
    const SlopingSeabedMesh& seabedMesh,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true)
{
    glm::vec3 sandColor = glm::vec3(0.85f, 0.78f, 0.67f); // Natural coastal quartz & silica sand

    if (texMgr && enableTextures)
    {
        texMgr->bind(TEX_SAND);
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 1);
        glUniform1f(glGetUniformLocation(shader.ID, "texScale"), 18.0f);
    }
    // GOURAUD SHADING for smooth sloping coastal bathymetry
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 1);

    glm::mat4 model = glm::mat4(1.0f);
    GLuint modelLoc = glGetUniformLocation(shader.ID, "model");
    GLuint colorLoc = glGetUniformLocation(shader.ID, "objectColor");
    GLuint specLoc  = glGetUniformLocation(shader.ID, "specularStrength");
    GLuint shinLoc  = glGetUniformLocation(shader.ID, "shininess");
    GLuint unlitLoc = glGetUniformLocation(shader.ID, "unlit");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(sandColor));
    glUniform1f(specLoc, 0.06f);
    glUniform1f(shinLoc, 8.0f);
    glUniform1i(unlitLoc, 0);

    seabedMesh.draw();

    if (texMgr && enableTextures)
    {
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 0);
        texMgr->unbind();
    }

    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 0);
}

// ============================================================================
// OBJECT 7D: 3D WATER DEPTH CROSS-SECTION SKIRT (Diagram A)
// Cut-away volume side walls showing depth thickness of coastal water column
// ============================================================================
inline void drawWaterDepthSkirt(
    Shader& shader,
    const WaterDepthSkirtMesh& skirtMesh)
{
    // Deep ocean cross-section cutaway wall color (Diagram A block diagram)
    glm::vec3 cutawayColor = glm::vec3(0.04f, 0.22f, 0.40f);

    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 0); // Phong
    glm::mat4 model = glm::mat4(1.0f);
    GLuint modelLoc = glGetUniformLocation(shader.ID, "model");
    GLuint colorLoc = glGetUniformLocation(shader.ID, "objectColor");
    GLuint specLoc  = glGetUniformLocation(shader.ID, "specularStrength");
    GLuint shinLoc  = glGetUniformLocation(shader.ID, "shininess");
    GLuint unlitLoc = glGetUniformLocation(shader.ID, "unlit");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(cutawayColor));
    glUniform1f(specLoc, 0.25f);
    glUniform1f(shinLoc, 32.0f);
    glUniform1i(unlitLoc, 0);

    skirtMesh.draw();
}

// ============================================================================
// OBJECT 7E: COASTAL TIMBER JETTY / GROIN (Diagrams A & B)
// Marine timber jetty structure with pilings extending into the surf zone
// ============================================================================
inline void drawCoastalJetty(
    Shader& shader,
    const CubeMesh& cube,
    const glm::vec3& jettyBasePos,
    float length = 18.0f,
    float width = 2.4f,
    float rotY = 0.0f)
{
    glm::vec3 deckWoodColor = glm::vec3(0.58f, 0.44f, 0.30f); // Weathered timber decking
    glm::vec3 postWoodColor = glm::vec3(0.36f, 0.27f, 0.18f); // Wet submerged marine pilings
    glm::vec3 beamWoodColor = glm::vec3(0.48f, 0.36f, 0.24f); // Cross stringers

    glm::mat4 baseMat = glm::translate(glm::mat4(1.0f), jettyBasePos);
    baseMat = glm::rotate(baseMat, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));

    // Deck elevation is at Y = +0.75f above still water level
    float deckY = 0.75f;
    float plankThickness = 0.12f;

    // 1. Pilings and Cross-beams placed every 2.8 units along length
    int numPiles = (int)(length / 2.8f) + 1;
    for (int i = 0; i < numPiles; ++i)
    {
        float zOff = (float)i * 2.8f;
        float worldZ = jettyBasePos.z + zOff;
        float seabedY = getCoastalSeabedElevation(worldZ);

        float pileHeight = (deckY + 0.15f) - (seabedY - 0.6f);
        float pileCenterY = (deckY + 0.15f + seabedY - 0.6f) * 0.5f;

        // Left piling
        glm::mat4 mPileL = glm::translate(baseMat, glm::vec3(-width * 0.42f, pileCenterY, zOff));
        mPileL = glm::scale(mPileL, glm::vec3(0.28f, pileHeight, 0.28f));
        drawTransformedCube(shader, cube, mPileL, postWoodColor, 12.0f, 0.10f, 0);

        // Right piling
        glm::mat4 mPileR = glm::translate(baseMat, glm::vec3(width * 0.42f, pileCenterY, zOff));
        mPileR = glm::scale(mPileR, glm::vec3(0.28f, pileHeight, 0.28f));
        drawTransformedCube(shader, cube, mPileR, postWoodColor, 12.0f, 0.10f, 0);

        // Transverse cross-beam capping the two pilings
        glm::mat4 mBeam = glm::translate(baseMat, glm::vec3(0.0f, deckY - 0.10f, zOff));
        mBeam = glm::scale(mBeam, glm::vec3(width * 0.95f, 0.20f, 0.25f));
        drawTransformedCube(shader, cube, mBeam, beamWoodColor, 16.0f, 0.12f, 0);
    }

    // 2. Longitudinal stringers supporting the deck planks
    glm::mat4 mStringerL = glm::translate(baseMat, glm::vec3(-width * 0.35f, deckY - 0.05f, length * 0.5f));
    mStringerL = glm::scale(mStringerL, glm::vec3(0.18f, 0.14f, length));
    drawTransformedCube(shader, cube, mStringerL, beamWoodColor, 16.0f, 0.12f, 0);

    glm::mat4 mStringerR = glm::translate(baseMat, glm::vec3(width * 0.35f, deckY - 0.05f, length * 0.5f));
    mStringerR = glm::scale(mStringerR, glm::vec3(0.18f, 0.14f, length));
    drawTransformedCube(shader, cube, mStringerR, beamWoodColor, 16.0f, 0.12f, 0);

    // 3. Transverse Timber Planks across the Deck
    int numPlanks = (int)(length / 0.45f);
    for (int p = 0; p < numPlanks; ++p)
    {
        float zOff = (float)p * 0.45f + 0.22f;
        glm::vec3 plankColor = deckWoodColor * (0.92f + 0.16f * ((p % 3) == 0 ? 1.0f : ((p % 2) == 0 ? 0.5f : 0.0f)));
        glm::mat4 mPlank = glm::translate(baseMat, glm::vec3(0.0f, deckY, zOff));
        mPlank = glm::scale(mPlank, glm::vec3(width, plankThickness, 0.38f));
        drawTransformedCube(shader, cube, mPlank, plankColor, 20.0f, 0.15f, 0);
    }
}

// ============================================================================
// OBJECT 8: SAND BEACH / SHORELINE
// Expansive coastal ground plane extending broadly across the bay.
// Rendered with Gouraud Shading and procedural sand texture mapping.
// ============================================================================
inline void drawBeach(
    Shader& shader,
    const CubeMesh& cube,
    const glm::vec3& beachCenter = glm::vec3(0.0f, -0.1f, -10.0f),
    float width = 350.0f,
    float depth = 45.0f,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true)
{
    glm::vec3 sandColor = glm::vec3(0.85f, 0.78f, 0.67f); // Natural coastal sand

    if (texMgr && enableTextures)
    {
        texMgr->bind(TEX_SAND);
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 1);
        glUniform1f(glGetUniformLocation(shader.ID, "texScale"), 20.0f);
    }
    // GOURAUD SHADING for smooth color interpolation across sand terrain
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 1);

    glm::mat4 sandMat = glm::translate(glm::mat4(1.0f), beachCenter);
    sandMat = glm::scale(sandMat, glm::vec3(width, 0.25f, depth));

    // Sand has low specular reflection (matte)
    drawTransformedCube(shader, cube, sandMat, sandColor, 8.0f, 0.08f, 0);

    if (texMgr && enableTextures)
    {
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 0);
        texMgr->unbind();
    }

    // Restore to default Phong
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 0);
}

// ============================================================================
// OBJECT 8B: COASTAL DUNES, HILLS, AND HEADLAND BLUFFS
// Replaces the "floating ship" void with a complete, realistic coastal landscape:
// rolling sand dunes, grassy bluffs, and rocky headlands framing the bay.
// ============================================================================
inline void drawCoastalDunesAndHills(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere)
{
    glm::vec3 duneSandColor  = glm::vec3(0.84f, 0.77f, 0.66f); // Coastal silica dune sand
    glm::vec3 duneGrassColor = glm::vec3(0.42f, 0.48f, 0.28f); // Coastal marram grass / dune scrub
    glm::vec3 hillGreenColor = glm::vec3(0.24f, 0.42f, 0.18f); // Deep coastal ridge foliage
    glm::vec3 cliffRockColor = glm::vec3(0.48f, 0.45f, 0.40f); // Coastal headland bluff rock

    // 1. REAR COASTAL SAND DUNES (Placed well back at Z = -24 to -35 to open the expansive beach)
    float duneData[] = {
        // x,       y,     z,      scaleX, scaleY, scaleZ, rotY
        -160.0f,  0.8f, -28.0f,    38.0f,  3.2f,   22.0f,  12.0f,
        -110.0f,  0.9f, -26.0f,    35.0f,  3.5f,   20.0f,  -8.0f,
         -70.0f,  0.8f, -25.0f,    32.0f,  3.0f,   18.0f,  15.0f,
         -35.0f,  0.7f, -24.0f,    28.0f,  2.8f,   16.0f, -10.0f,
           0.0f,  0.8f, -25.0f,    32.0f,  3.0f,   18.0f,   5.0f,
          35.0f,  0.7f, -24.0f,    28.0f,  2.8f,   16.0f, -12.0f,
          70.0f,  0.8f, -25.0f,    32.0f,  3.0f,   18.0f,  10.0f,
         110.0f,  0.9f, -26.0f,    35.0f,  3.5f,   20.0f,  -8.0f,
         160.0f,  0.8f, -28.0f,    38.0f,  3.2f,   22.0f,  14.0f
    };

    for (int i = 0; i < 9; ++i)
    {
        float* d = &duneData[i * 7];
        float groundY = getCoastalTerrainElevation(d[0], d[2]);
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(d[0], groundY + 0.3f, d[2]));
        m = glm::rotate(m, glm::radians(d[6]), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(d[3], d[4], d[5]));
        drawTransformedSphere(shader, sphere, m, (i % 2 == 0) ? duneSandColor : duneGrassColor, 12.0f, 0.08f, 0);
    }

    // 2. HIGHER COASTAL RIDGE / BLUFF HILLS IN BACKGROUND (Z = -38 to -65)
    float hillData[] = {
        // x,       y,     z,      scaleX, scaleY, scaleZ, rotY
        -180.0f,  3.5f, -45.0f,    65.0f,  10.0f,  38.0f,   8.0f,
        -100.0f,  4.2f, -48.0f,    70.0f,  12.0f,  42.0f,  -5.0f,
         -30.0f,  4.5f, -46.0f,    75.0f,  13.0f,  44.0f,   6.0f,
          40.0f,  4.2f, -48.0f,    72.0f,  12.5f,  40.0f, -10.0f,
         110.0f,  3.8f, -46.0f,    68.0f,  11.0f,  38.0f,  12.0f,
         180.0f,  3.5f, -45.0f,    65.0f,  10.0f,  38.0f, -14.0f,
           0.0f,  5.8f, -65.0f,   110.0f,  16.0f,  50.0f,   0.0f
    };

    for (int i = 0; i < 7; ++i)
    {
        float* h = &hillData[i * 7];
        float groundY = getCoastalTerrainElevation(h[0], h[2]);
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(h[0], groundY + 1.2f, h[2]));
        m = glm::rotate(m, glm::radians(h[6]), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(h[3], h[4], h[5]));
        drawTransformedSphere(shader, sphere, m, (i == 6) ? cliffRockColor : hillGreenColor, 8.0f, 0.05f, 0);
    }

    // 3. DISTANT COASTAL HEADLANDS (Far distant headlands framing the vast 1.2km bay at X = +/- 180 to 260m)
    for (int i = 0; i < 4; ++i)
    {
        float zPos = -42.0f - (float)i * 12.0f;
        float xPos = -160.0f - (float)i * 28.0f;
        float h = 6.5f + (float)i * 2.0f;
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, h * 0.45f, zPos));
        m = glm::scale(m, glm::vec3(45.0f, h, 35.0f));
        drawTransformedSphere(shader, sphere, m, (i % 2 == 0) ? hillGreenColor : cliffRockColor, 12.0f, 0.08f, 0);
    }

    for (int i = 0; i < 4; ++i)
    {
        float zPos = -42.0f - (float)i * 12.0f;
        float xPos = 160.0f + (float)i * 28.0f;
        float h = 6.5f + (float)i * 2.0f;
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, h * 0.45f, zPos));
        m = glm::scale(m, glm::vec3(45.0f, h, 35.0f));
        drawTransformedSphere(shader, sphere, m, (i % 2 == 0) ? hillGreenColor : cliffRockColor, 12.0f, 0.08f, 0);
    }
}

// ============================================================================
// OBJECT 8C: BACKDROP PALM FOREST (LUSH HORIZON ENCLOSURE)
// Natural palm groves lining the back of the dunes across the entire 1.2km coast.
// Leaves the beach shoreline completely open and infinite.
// ============================================================================
inline void drawBackgroundPalmForest(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    float windSway = 0.0f,
    const TextureManager* texMgr = nullptr,
    bool enableTextures = true)
{
    struct BackdropTree {
        float x, z;
        float scale;
        float trunkCurve;
        float rotY;
    };

    const BackdropTree bgTrees[] = {
        // Far West Coast Palms (X = -180 to -60, Z = -22 to -30)
        { -180.0f, -28.0f, 1.35f,  3.2f,  25.0f },
        { -150.0f, -25.0f, 1.25f, -2.8f, -15.0f },
        { -125.0f, -24.0f, 1.30f,  3.5f,  40.0f },
        { -100.0f, -23.0f, 1.20f, -3.0f, -20.0f },
        {  -80.0f, -25.0f, 1.40f,  3.8f,  35.0f },
        {  -60.0f, -23.0f, 1.25f, -2.5f, -30.0f },
        {  -45.0f, -24.0f, 1.15f,  3.0f,  15.0f },

        // Central Rear Dune Grove (X = -32 to +32, Z = -24 to -30)
        {  -32.0f, -26.0f, 1.30f, -3.2f,  50.0f },
        {  -22.0f, -25.0f, 1.25f,  3.4f, -25.0f },
        {  -12.0f, -26.0f, 1.35f, -3.6f,  60.0f },
        {   -4.0f, -25.0f, 1.40f,  3.8f,  25.0f },
        {    6.0f, -26.0f, 1.30f,  3.2f,  35.0f },
        {   16.0f, -24.0f, 1.20f, -2.8f, -40.0f },
        {   28.0f, -25.0f, 1.35f,  3.6f,  20.0f },

        // Far East Coast Palms (X = +45 to +185, Z = -22 to -30)
        {   45.0f, -24.0f, 1.25f, -3.0f, -15.0f },
        {   65.0f, -23.0f, 1.35f,  3.5f,  30.0f },
        {   85.0f, -25.0f, 1.20f, -2.6f, -35.0f },
        {  110.0f, -24.0f, 1.40f,  3.8f,  15.0f },
        {  135.0f, -26.0f, 1.30f, -3.4f,  45.0f },
        {  160.0f, -25.0f, 1.25f,  2.9f, -20.0f },
        {  185.0f, -28.0f, 1.35f, -3.5f,  35.0f },

        // Elevated Deep Background Ridge Palms (Z = -40 to -52)
        { -120.0f, -42.0f, 1.55f,  3.0f,  10.0f },
        {  -60.0f, -40.0f, 1.60f, -3.5f, -30.0f },
        {    0.0f, -44.0f, 1.65f,  4.0f,  15.0f },
        {   60.0f, -40.0f, 1.55f, -3.2f, -25.0f },
        {  120.0f, -42.0f, 1.50f,  3.5f,  40.0f }
    };

    for (const auto& t : bgTrees)
    {
        float groundY = getCoastalTerrainElevation(t.x, t.z);
        glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(t.x, groundY, t.z));
        m = glm::rotate(m, glm::radians(t.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(t.scale));
        drawPalmTree(shader, cube, sphere, m, t.trunkCurve, windSway, texMgr, enableTextures);
    }
}

// ============================================================================
// OBJECT 8D: SAINT MARTIN-INSPIRED ISLAND BACKDROP
// The ground is a triangulated heightfield (IslandTerrainMesh), never scaled
// spheres. A dense irregular treeline follows the computed slopes.
// ============================================================================
inline void drawIslandTerrainBackdrop(
    Shader& shader,
    const IslandTerrainMesh& terrain,
    const CubeMesh& cube,
    const SphereMesh& sphere)
{
    const glm::vec3 jungleDark(0.055f, 0.25f, 0.10f);
    const glm::vec3 jungleMid(0.08f, 0.38f, 0.13f);
    const glm::vec3 jungleSun(0.16f, 0.50f, 0.18f);

    glUniform1i(glGetUniformLocation(shader.ID, "terrainPalette"), 1);
    glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), 0);
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 0);
    glm::mat4 identity(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(identity));
    glm::vec3 baseColor(0.28f, 0.34f, 0.16f);
    glUniform3fv(glGetUniformLocation(shader.ID, "objectColor"), 1, glm::value_ptr(baseColor));
    glUniform1f(glGetUniformLocation(shader.ID, "specularStrength"), 0.04f);
    glUniform1f(glGetUniformLocation(shader.ID, "shininess"), 7.0f);
    glUniform1i(glGetUniformLocation(shader.ID, "unlit"), 0);
    terrain.draw();
    glUniform1i(glGetUniformLocation(shader.ID, "terrainPalette"), 0);

    // Distant broadleaf trees sit on the mesh rather than floating in front of
    // it. Three offset crown lobes prevent a row of identical green balls.
    for (int i = 0; i < 180; ++i)
    {
        float x = -105.0f + (float)((i * 73) % 211);
        float z = 25.0f - (float)((i * 97) % 255);
        if (getIslandSignedDistance(x, z) < 27.0f)
            continue;
        // Keep a readable clearing around the elevated lighthouse compound.
        float lighthouseDx = x + 6.0f;
        float lighthouseDz = z + 110.0f;
        if (lighthouseDx * lighthouseDx + lighthouseDz * lighthouseDz < 15.0f * 15.0f)
            continue;
        float baseY = getIslandBackdropElevation(x, z);
        float h = 1.9f + (float)((i * 7) % 6) * 0.25f;
        float crownScale = 1.05f + (float)((i * 11) % 7) * 0.08f;

        glm::mat4 trunk = glm::translate(glm::mat4(1.0f), glm::vec3(x, baseY + h * 0.5f, z));
        trunk = glm::rotate(trunk, glm::radians((float)((i * 13) % 9) - 4.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        trunk = glm::scale(trunk, glm::vec3(0.24f, h, 0.24f));
        drawTransformedCube(shader, cube, trunk, glm::vec3(0.24f, 0.16f, 0.09f), 5.0f, 0.02f);

        for (int lobe = 0; lobe < 4; ++lobe)
        {
            const float offsets[4][3] = {
                { -0.72f, 0.05f,  0.18f }, { 0.66f, 0.12f, 0.05f },
                {  0.02f, 0.68f, -0.24f }, { 0.10f, 0.18f, 0.62f }
            };
            float ox = offsets[lobe][0] * crownScale;
            float oy = offsets[lobe][1] * crownScale;
            float oz = offsets[lobe][2] * crownScale;
            glm::mat4 crown = glm::translate(glm::mat4(1.0f), glm::vec3(x + ox, baseY + h + oy, z + oz));
            crown = glm::rotate(crown, glm::radians((float)((i * 19 + lobe * 31) % 40)), glm::vec3(0.0f, 1.0f, 0.0f));
            crown = glm::scale(crown, glm::vec3(1.25f, 1.40f, 1.15f) * crownScale);
            glm::vec3 leaf = (lobe == 1) ? jungleSun : ((i % 3 == 0) ? jungleDark : jungleMid);
            drawTransformedSphere(shader, sphere, crown, leaf, 5.0f, 0.02f);
        }
    }
}

// ============================================================================
// OBJECT 8E: BEACH 4WD
// A compact, high-clearance island utility vehicle with four visible tyres,
// bumpers, cabin glazing, roof rack and rear spare wheel.
// ============================================================================
inline void drawBeach4WD(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const FrustumMesh& wheelMesh,
    const glm::mat4& baseTransform)
{
    const glm::vec3 body(0.58f, 0.55f, 0.46f);      // Sand/camouflage bodywork
    const glm::vec3 bodyDark(0.11f, 0.12f, 0.12f);  // Black lower cladding
    const glm::vec3 rubber(0.035f, 0.04f, 0.04f);
    const glm::vec3 rim(0.20f, 0.22f, 0.22f);
    const glm::vec3 seatColor(0.075f, 0.08f, 0.085f);

    // Compact quad-bike proportions: 2.45 m long, 1.28 m wide.
    glm::mat4 chassis = glm::translate(baseTransform, glm::vec3(0.0f, 0.62f, 0.0f));
    chassis = glm::scale(chassis, glm::vec3(2.05f, 0.30f, 0.82f));
    drawTransformedCube(shader, cube, chassis, bodyDark, 42.0f, 0.35f);

    glm::mat4 frontBody = glm::translate(baseTransform, glm::vec3(0.70f, 0.88f, 0.0f));
    frontBody = glm::rotate(frontBody, glm::radians(-7.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    frontBody = glm::scale(frontBody, glm::vec3(0.95f, 0.48f, 1.02f));
    drawTransformedCube(shader, cube, frontBody, body, 45.0f, 0.32f);

    glm::mat4 rearBody = glm::translate(baseTransform, glm::vec3(-0.72f, 0.86f, 0.0f));
    rearBody = glm::scale(rearBody, glm::vec3(0.78f, 0.42f, 1.04f));
    drawTransformedCube(shader, cube, rearBody, body, 45.0f, 0.32f);

    // Saddle and full-height passenger backrest from the reference ATV.
    glm::mat4 seat = glm::translate(baseTransform, glm::vec3(-0.18f, 1.15f, 0.0f));
    seat = glm::scale(seat, glm::vec3(1.05f, 0.20f, 0.62f));
    drawTransformedCube(shader, cube, seat, seatColor, 22.0f, 0.16f);
    glm::mat4 backrest = glm::translate(baseTransform, glm::vec3(-0.82f, 1.62f, 0.0f));
    backrest = glm::rotate(backrest, glm::radians(-8.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    backrest = glm::scale(backrest, glm::vec3(0.20f, 0.92f, 0.72f));
    drawTransformedCube(shader, cube, backrest, seatColor, 22.0f, 0.16f);

    // Four large knobby cylindrical tyres and dark alloy hubs.
    const float wheelX[2] = { -0.78f, 0.78f };
    for (int axle = 0; axle < 2; ++axle)
    {
        for (int side = -1; side <= 1; side += 2)
        {
            glm::mat4 tyre = glm::translate(baseTransform, glm::vec3(wheelX[axle], 0.50f, side * 0.72f));
            tyre = glm::rotate(tyre, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            tyre = glm::scale(tyre, glm::vec3(0.88f, 0.34f, 0.88f));
            drawTransformedFrustum(shader, wheelMesh, tyre, rubber, 18.0f, 0.12f);

            glm::mat4 hub = glm::translate(baseTransform, glm::vec3(wheelX[axle], 0.50f, side * 0.90f));
            hub = glm::rotate(hub, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            hub = glm::scale(hub, glm::vec3(0.42f, 0.05f, 0.42f));
            drawTransformedFrustum(shader, wheelMesh, hub, rim, 72.0f, 0.65f);
        }
    }

    // Front/rear cargo racks with transverse rails.
    const float rackX[2] = { -0.92f, 0.93f };
    for (int rack = 0; rack < 2; ++rack)
    {
        glm::mat4 rackBase = glm::translate(baseTransform, glm::vec3(rackX[rack], 1.16f, 0.0f));
        rackBase = glm::scale(rackBase, glm::vec3(0.62f, 0.07f, 1.15f));
        drawTransformedCube(shader, cube, rackBase, bodyDark, 18.0f, 0.14f);
    }
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 rail = glm::translate(baseTransform, glm::vec3(0.0f, 1.21f, side * 0.54f));
        rail = glm::scale(rail, glm::vec3(2.30f, 0.07f, 0.07f));
        drawTransformedCube(shader, cube, rail, rubber, 18.0f, 0.15f);
    }

    // Steering column, handlebars, mirrors, and front brush guard.
    glm::mat4 column = glm::translate(baseTransform, glm::vec3(0.42f, 1.48f, 0.0f));
    column = glm::rotate(column, glm::radians(-18.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    column = glm::scale(column, glm::vec3(0.10f, 0.82f, 0.10f));
    drawTransformedCube(shader, cube, column, bodyDark, 18.0f, 0.16f);
    glm::mat4 handlebar = glm::translate(baseTransform, glm::vec3(0.28f, 1.84f, 0.0f));
    handlebar = glm::scale(handlebar, glm::vec3(0.12f, 0.10f, 1.05f));
    drawTransformedCube(shader, cube, handlebar, bodyDark, 18.0f, 0.16f);

    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 mirrorStem = glm::translate(baseTransform, glm::vec3(0.27f, 2.06f, side * 0.43f));
        mirrorStem = glm::rotate(mirrorStem, glm::radians((float)-side * 18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        mirrorStem = glm::scale(mirrorStem, glm::vec3(0.055f, 0.52f, 0.055f));
        drawTransformedCube(shader, cube, mirrorStem, bodyDark, 18.0f, 0.16f);
        glm::mat4 mirror = glm::translate(baseTransform, glm::vec3(0.27f, 2.31f, side * 0.52f));
        mirror = glm::scale(mirror, glm::vec3(0.14f, 0.24f, 0.06f));
        drawTransformedSphere(shader, sphere, mirror, bodyDark, 64.0f, 0.55f);

        glm::mat4 lamp = glm::translate(baseTransform, glm::vec3(1.20f, 0.94f, side * 0.28f));
        lamp = glm::scale(lamp, glm::vec3(0.10f, 0.20f, 0.24f));
        drawTransformedSphere(shader, sphere, lamp, glm::vec3(1.0f, 0.88f, 0.52f), 64.0f, 0.65f, 1);
    }
    glm::mat4 guard = glm::translate(baseTransform, glm::vec3(1.27f, 0.68f, 0.0f));
    guard = glm::scale(guard, glm::vec3(0.10f, 0.55f, 1.05f));
    drawTransformedCube(shader, cube, guard, bodyDark, 18.0f, 0.15f);
}

// ============================================================================
// OBJECT 8F: COASTAL LIGHTHOUSE
// Banded masonry tower, glazed lantern room, gallery, roof and glowing optic.
// ============================================================================
inline void drawLighthouse(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const FrustumMesh& towerMesh,
    const FrustumMesh& cylinderMesh,
    const FrustumMesh& roofMesh,
    const glm::mat4& baseTransform,
    float time)
{
    const glm::vec3 whiteStone(0.88f, 0.86f, 0.78f);
    const glm::vec3 redBand(0.72f, 0.07f, 0.055f);
    const glm::vec3 darkMetal(0.10f, 0.12f, 0.12f);
    const glm::vec3 lanternGlass(0.20f, 0.50f, 0.58f);

    // 14 m tapered masonry shaft: 4.6 m diameter at the base, 3.1 m at top.
    glm::mat4 tower = glm::translate(baseTransform, glm::vec3(0.0f, 7.0f, 0.0f));
    tower = glm::scale(tower, glm::vec3(4.6f, 14.0f, 4.6f));
    drawTransformedFrustum(shader, towerMesh, tower, whiteStone, 24.0f, 0.18f);

    // Painted red daymark bands sit slightly proud of the masonry surface.
    for (int band = 0; band < 2; ++band)
    {
        float y = (band == 0) ? 4.4f : 10.2f;
        float diameter = (band == 0) ? 4.18f : 3.55f;
        glm::mat4 stripe = glm::translate(baseTransform, glm::vec3(0.0f, y, 0.0f));
        stripe = glm::scale(stripe, glm::vec3(diameter, 1.35f, diameter));
        drawTransformedFrustum(shader, cylinderMesh, stripe, redBand, 26.0f, 0.20f);
    }

    // Human scale: 1.05 m wide x 2.25 m high entrance.
    glm::mat4 door = glm::translate(baseTransform, glm::vec3(0.0f, 1.13f, 2.24f));
    door = glm::scale(door, glm::vec3(1.05f, 2.25f, 0.12f));
    drawTransformedCube(shader, cube, door, darkMetal, 28.0f, 0.20f);

    for (int level = 0; level < 2; ++level)
    {
        float y = 6.7f + (float)level * 4.0f;
        float radius = 1.94f - (float)level * 0.22f;
        glm::mat4 window = glm::translate(baseTransform, glm::vec3(0.0f, y, radius));
        window = glm::scale(window, glm::vec3(0.58f, 0.92f, 0.10f));
        drawTransformedCube(shader, cube, window, lanternGlass, 72.0f, 0.65f);
    }

    glm::mat4 gallery = glm::translate(baseTransform, glm::vec3(0.0f, 14.15f, 0.0f));
    gallery = glm::scale(gallery, glm::vec3(5.4f, 0.24f, 5.4f));
    drawTransformedFrustum(shader, cylinderMesh, gallery, darkMetal, 42.0f, 0.35f);

    glm::mat4 lantern = glm::translate(baseTransform, glm::vec3(0.0f, 15.45f, 0.0f));
    lantern = glm::scale(lantern, glm::vec3(3.05f, 2.35f, 3.05f));
    drawTransformedFrustum(shader, cylinderMesh, lantern, lanternGlass, 96.0f, 0.85f);

    // Gallery handrail: about 1.05 m tall, with twelve evenly spaced posts.
    for (int i = 0; i < 12; ++i)
    {
        float a = (float)i * 30.0f;
        float r = 2.45f;
        glm::mat4 post = glm::translate(baseTransform,
            glm::vec3(cosf(glm::radians(a)) * r, 14.72f, sinf(glm::radians(a)) * r));
        post = glm::scale(post, glm::vec3(0.075f, 1.05f, 0.075f));
        drawTransformedCube(shader, cube, post, darkMetal, 18.0f, 0.18f);
    }
    glm::mat4 handrail = glm::translate(baseTransform, glm::vec3(0.0f, 15.23f, 0.0f));
    handrail = glm::scale(handrail, glm::vec3(5.05f, 0.10f, 5.05f));
    drawTransformedFrustum(shader, cylinderMesh, handrail, darkMetal, 28.0f, 0.28f);

    glm::mat4 roof = glm::translate(baseTransform, glm::vec3(0.0f, 17.30f, 0.0f));
    roof = glm::scale(roof, glm::vec3(3.65f, 1.45f, 3.65f));
    drawTransformedFrustum(shader, roofMesh, roof, redBand, 38.0f, 0.35f);

    // The lamp itself is steady; the rotating focused optic creates the flash.
    float pulse = 1.0f;
    glm::mat4 optic = glm::translate(baseTransform, glm::vec3(0.0f, 15.55f, 0.0f));
    optic = glm::scale(optic, glm::vec3(0.62f * pulse));
    drawTransformedSphere(shader, sphere, optic, glm::vec3(1.0f, 0.82f, 0.24f), 1.0f, 0.0f, 1);
}

// Rotating lighthouse beacon. The cone is additive and does not write depth,
// so it reads as illuminated mist while still being occluded by solid terrain.
inline void drawLighthouseBeacon(
    Shader& shader,
    const FrustumMesh& coneMesh,
    const glm::mat4& lighthouseTransform,
    float time,
    float nightIntensity)
{
    if (nightIntensity <= 0.01f)
        return;

    const float beamLength = 300.0f;
    // One revolution in roughly 6.5 seconds, producing a recognizable flash
    // characteristic whenever the narrow beam passes an observer.
    float sweep = time * 55.0f;
    glm::mat4 beam = glm::translate(lighthouseTransform, glm::vec3(0.0f, 15.65f, 0.0f));
    beam = glm::rotate(beam, glm::radians(sweep), glm::vec3(0.0f, 1.0f, 0.0f));
    beam = glm::translate(beam, glm::vec3(beamLength * 0.5f, 0.0f, 0.0f));
    beam = glm::rotate(beam, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glUniform1i(glGetUniformLocation(shader.ID, "beaconBeamPass"), 1);

    // The optic has small vertical divergence but a somewhat wider horizontal
    // fan. This avoids the previous pair of thick, parallel luminous rails.
    glUniform1f(glGetUniformLocation(shader.ID, "objectAlpha"), 0.030f * nightIntensity);
    glm::mat4 halo = glm::scale(beam, glm::vec3(3.0f, beamLength, 22.0f));
    drawTransformedFrustum(shader, coneMesh, halo, glm::vec3(0.72f, 0.84f, 1.0f), 1.0f, 0.0f, 1);

    glUniform1f(glGetUniformLocation(shader.ID, "objectAlpha"), 0.10f * nightIntensity);
    glm::mat4 core = glm::scale(beam, glm::vec3(1.15f, beamLength, 8.0f));
    drawTransformedFrustum(shader, coneMesh, core, glm::vec3(1.0f, 0.96f, 0.74f), 1.0f, 0.0f, 1);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform1f(glGetUniformLocation(shader.ID, "objectAlpha"), 1.0f);
    glUniform1i(glGetUniformLocation(shader.ID, "beaconBeamPass"), 0);
}

// Deterministic staggered falling-and-bouncing coconuts. Each coconut spends
// most of its cycle attached, then drops with gravity, bounces once, and resets.
inline void drawFallingCoconuts(
    Shader& shader,
    const SphereMesh& sphere,
    float time)
{
    struct Drop { glm::vec3 crown; float phase; float drift; };
    const Drop drops[] = {
        { glm::vec3(-29.0f, 6.20f, 29.0f), 0.4f,  0.22f },
        { glm::vec3(-14.5f, 6.75f, 23.0f), 3.1f, -0.18f },
        { glm::vec3( 17.8f, 7.05f, 24.0f), 6.0f,  0.16f },
        { glm::vec3( 33.0f, 7.25f, 28.0f), 8.8f, -0.20f }
    };
    const float cycle = 12.0f;
    const float fallStart = 7.0f;
    for (const auto& d : drops)
    {
        float local = fmodf(time + d.phase, cycle);
        float ground = getCoastalTerrainElevation(d.crown.x, d.crown.z) + 0.23f;
        float y = d.crown.y;
        float x = d.crown.x;
        float spin = 0.0f;
        if (local > fallStart)
        {
            float t = local - fallStart;
            float rawY = d.crown.y - 4.9f * t * t;
            y = glm::max(rawY, ground);
            x += d.drift * t;
            spin = t * 310.0f;
            if (rawY < ground)
            {
                float impactT = sqrtf(glm::max((d.crown.y - ground) / 4.9f, 0.0f));
                float bounceT = t - impactT;
                y = ground + glm::max(0.0f, 1.45f * bounceT - 2.9f * bounceT * bounceT);
            }
        }

        glm::mat4 coconut = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, d.crown.z));
        coconut = glm::rotate(coconut, glm::radians(spin), glm::vec3(0.0f, 0.0f, 1.0f));
        coconut = glm::scale(coconut, glm::vec3(0.34f, 0.42f, 0.34f));
        drawTransformedSphere(shader, sphere, coconut, glm::vec3(0.25f, 0.13f, 0.055f), 18.0f, 0.12f);
    }
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
    // FLAT SHADING for faceted coastal rock formations
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 2);

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

    // Restore to default Phong
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 0);
}

// Organic standalone rock cluster positioned relative to baseTransform (ideal for showcase)
inline void drawRockCluster(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform)
{
    // FLAT SHADING for faceted rock cluster
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 2);

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

    // Restore to default Phong
    glUniform1i(glGetUniformLocation(shader.ID, "shadingMode"), 0);
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
