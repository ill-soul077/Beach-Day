#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "MeshPrimitives.h"
#include "SceneObjects.h"
#include "Camera.h"

// Window Dimensions
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// Camera
Camera camera(glm::vec3(0.0f, 1.2f, 0.0f), 11.0f);

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// User editable variables for live viva demonstration
float g_robotHeadYaw     = 15.0f;
float g_robotLeftArm     = -20.0f;
float g_robotRightArm    = 30.0f;
float g_umbrellaTilt     = 14.0f;
float g_palmTrunkCurve   = 3.2f;
bool  g_wireframe        = false;

// Callback Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

int main()
{
    // 1. Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Create Window
    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH, SCR_HEIGHT,
        "Beach Day: 3D Coastal Scene (Object Modeling) - CSE 4102",
        NULL, NULL
    );

    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Register Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 3. Load GLAD Function Pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    std::cout << "=======================================================\n";
    std::cout << "  BEACH DAY: 3D OBJECT MODELING (CSE 4102 LABORATORY)\n";
    std::cout << "  Student: Hassan Mohammed Naquibul Hoque | Roll: 2107077\n";
    std::cout << "=======================================================\n";
    std::cout << "  Camera Controls:\n";
    std::cout << "    [Left Mouse Drag]  : Orbit around the scene\n";
    std::cout << "    [Right Mouse Drag] : Pan camera target\n";
    std::cout << "    [Scroll / W / S]   : Zoom in / Zoom out\n";
    std::cout << "    [A / D]            : Rotate around scene\n";
    std::cout << "    [R]                : Reset camera view\n";
    std::cout << "    [Z]                : Toggle Wireframe mode\n";
    std::cout << "    [1 / 2]            : Turn Robot Head Left/Right\n";
    std::cout << "    [3 / 4]            : Adjust Umbrella Tilt Angle\n";
    std::cout << "=======================================================\n";

    // 4. Build and Compile Shader Program
    Shader shaderProgram("default.vert", "default.frag");

    // 5. Instantiate Custom Procedural Base Primitives
    // (Zero GLUT/GLU objects - purely transformed unit cube & procedural sphere)
    CubeMesh cubeMesh;
    SphereMesh sphereMesh(24, 32, 0.5f);
    CanopyGoreMesh canopyGoreMesh;

    // Sun & Light Settings
    glm::vec3 sunPosition(15.0f, 22.0f, 28.0f);
    glm::vec3 sunLightColor(1.0f, 0.98f, 0.90f);

    // 6. Main Render Loop
    while (!glfwWindowShouldClose(window))
    {
        // Timing
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process Input
        processInput(window);
        camera.handleKeyboard(window, deltaTime);

        // Clear Color (Sky Blue)
        glClearColor(0.48f, 0.74f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.Activate();

        // View and Projection Matrices
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT);
        glm::vec3 camPos = camera.getPosition();

        GLuint viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
        GLuint viewPosLoc = glGetUniformLocation(shaderProgram.ID, "viewPos");
        GLuint lightDirLoc = glGetUniformLocation(shaderProgram.ID, "lightDir");
        GLuint lightColLoc = glGetUniformLocation(shaderProgram.ID, "lightColor");
        GLuint ambLoc = glGetUniformLocation(shaderProgram.ID, "ambientStrength");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camPos));

        // Directional Light pointing from Sun position
        glm::vec3 lightDir = glm::normalize(sunPosition);
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
        glUniform3fv(lightColLoc, 1, glm::value_ptr(sunLightColor));
        glUniform1f(ambLoc, 0.35f);

        // ====================================================================
        // RENDER SCENE OBJECTS
        // ====================================================================

        // 1. Sun (Glowing sphere in the sky)
        drawSun(shaderProgram, sphereMesh, sunPosition, 2.5f, glm::vec3(1.0f, 0.92f, 0.38f));

        // 2. Sand Beach Ground Plane
        drawBeach(shaderProgram, cubeMesh, glm::vec3(0.0f, -0.1f, -4.0f), 70.0f, 24.0f);

        // 3. Sea Water Plane
        drawSea(shaderProgram, cubeMesh, glm::vec3(0.0f, -0.12f, 22.0f), 70.0f, 40.0f);

        // 4. Coastal Boulders / Rocks (composite uneven shapes)
        drawRocks(shaderProgram, cubeMesh, sphereMesh);

        // 5. Palm Trees (Framing both flanks and rear dunes with zero umbrella overlap)
        // Left Flank Palm Tree
        glm::mat4 leftPalmMat = glm::translate(glm::mat4(1.0f), glm::vec3(-14.0f, 0.0f, -4.5f));
        leftPalmMat = glm::rotate(leftPalmMat, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, leftPalmMat, g_palmTrunkCurve);

        // Rear Left Dune Palm Tree (placed further back)
        glm::mat4 backLeftPalm = glm::translate(glm::mat4(1.0f), glm::vec3(-4.5f, 0.0f, -8.0f));
        backLeftPalm = glm::rotate(backLeftPalm, glm::radians(70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        backLeftPalm = glm::scale(backLeftPalm, glm::vec3(1.10f));
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, backLeftPalm, -g_palmTrunkCurve * 0.75f);

        // Rear Right Dune Palm Tree (placed further back)
        glm::mat4 backRightPalm = glm::translate(glm::mat4(1.0f), glm::vec3(10.5f, 0.0f, -8.0f));
        backRightPalm = glm::rotate(backRightPalm, glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        backRightPalm = glm::scale(backRightPalm, glm::vec3(1.15f));
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, backRightPalm, g_palmTrunkCurve * 0.85f);

        // Right Flank Palm Tree
        glm::mat4 rightPalmMat = glm::translate(glm::mat4(1.0f), glm::vec3(19.5f, 0.0f, -4.0f));
        rightPalmMat = glm::rotate(rightPalmMat, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        rightPalmMat = glm::scale(rightPalmMat, glm::vec3(1.20f));
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, rightPalmMat, -g_palmTrunkCurve * 0.85f);

        // ========== BEACH SETUP 1: CENTER (Main interactive robot, X ~ 0) ==========
        // Chair 1
        glm::mat4 chairMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.8f));
        drawBeachChair(shaderProgram, cubeMesh, chairMat1);

        // Robot 1 (Interactive - head turns with keys 1/2)
        glm::mat4 robotMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.62f, -0.75f));
        drawRobot(shaderProgram, cubeMesh, sphereMesh, robotMat1,
            g_robotHeadYaw, -5.0f, g_robotLeftArm, g_robotRightArm, 82.0f);

        // Umbrella 1 (Interactive - tilts with keys 3/4)
        glm::mat4 umbrellaMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -1.5f));
        drawUmbrella(shaderProgram, cubeMesh, sphereMesh, canopyGoreMesh,
            umbrellaMat1, g_umbrellaTilt, 3.4f, 1.0f);

        // ========== BEACH SETUP 2: LEFT RESORT SPOT (X ~ -7.5) ==========
        // Chair 2
        glm::mat4 chairMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(-7.5f, 0.0f, -1.6f));
        chairMat2 = glm::rotate(chairMat2, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        drawBeachChair(shaderProgram, cubeMesh, chairMat2);

        // Robot 2 (Static pose - looking right, arms relaxed)
        glm::mat4 robotMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(-7.5f, 0.62f, -1.55f));
        robotMat2 = glm::rotate(robotMat2, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        drawRobot(shaderProgram, cubeMesh, sphereMesh, robotMat2,
            -25.0f, 5.0f, -10.0f, 15.0f, 82.0f);

        // Umbrella 2 (Well separated, distance ~7.6 units from Umbrella 1)
        glm::mat4 umbrellaMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(-8.6f, 0.0f, -2.4f));
        drawUmbrella(shaderProgram, cubeMesh, sphereMesh, canopyGoreMesh,
            umbrellaMat2, -8.0f, 3.2f, 0.95f);

        // ========== BEACH SETUP 3: RIGHT RESORT SPOT (X ~ +7.5) ==========
        // Chair 3
        glm::mat4 chairMat3 = glm::translate(glm::mat4(1.0f), glm::vec3(7.5f, 0.0f, -1.6f));
        chairMat3 = glm::rotate(chairMat3, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        drawBeachChair(shaderProgram, cubeMesh, chairMat3);

        // Robot 3 (Static pose - looking out to sea, arms up waving)
        glm::mat4 robotMat3 = glm::translate(glm::mat4(1.0f), glm::vec3(7.5f, 0.62f, -1.55f));
        robotMat3 = glm::rotate(robotMat3, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        drawRobot(shaderProgram, cubeMesh, sphereMesh, robotMat3,
            35.0f, -8.0f, -50.0f, -55.0f, 82.0f);

        // Umbrella 3 (Well separated, distance ~7.5 units from Umbrella 1)
        glm::mat4 umbrellaMat3 = glm::translate(glm::mat4(1.0f), glm::vec3(6.4f, 0.0f, -2.4f));
        drawUmbrella(shaderProgram, cubeMesh, sphereMesh, canopyGoreMesh,
            umbrellaMat3, 10.0f, 3.4f, 1.0f);

        // ========== BEACH SETUP 4: FAR RIGHT SUN LOUNGER (X ~ +15.0) ==========
        glm::mat4 chairMat4 = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, -2.2f));
        chairMat4 = glm::rotate(chairMat4, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        drawBeachChair(shaderProgram, cubeMesh, chairMat4);

        glm::mat4 umbrellaMat4 = glm::translate(glm::mat4(1.0f), glm::vec3(14.0f, 0.0f, -3.0f));
        drawUmbrella(shaderProgram, cubeMesh, sphereMesh, canopyGoreMesh,
            umbrellaMat4, -10.0f, 3.2f, 0.95f);

        // ========== OBJECT 10: LIFEGUARD WATCH TOWER ==========
        // Positioned on the shoreline overlooking the ocean (X = -4.2, Z = 3.2)
        glm::mat4 towerMat = glm::translate(glm::mat4(1.0f), glm::vec3(-4.2f, 0.0f, 3.2f));
        towerMat = glm::rotate(towerMat, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Slight angle to sea
        drawLifeguardTower(shaderProgram, cubeMesh, sphereMesh, towerMat);

        // ========== SAILBOATS ON THE SEA ==========
        // Boat 1 (Center-right, close)
        glm::mat4 boatMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(4.2f, 0.05f, 14.0f));
        boatMat1 = glm::rotate(boatMat1, glm::radians(-18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        boatMat1 = glm::rotate(boatMat1, glm::radians(3.5f), glm::vec3(1.0f, 0.0f, 0.0f));
        drawBoat(shaderProgram, cubeMesh, sphereMesh, boatMat1);

        // Boat 2 (Far left, distant)
        glm::mat4 boatMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, 0.03f, 22.0f));
        boatMat2 = glm::rotate(boatMat2, glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        boatMat2 = glm::rotate(boatMat2, glm::radians(-2.5f), glm::vec3(1.0f, 0.0f, 0.0f));
        boatMat2 = glm::scale(boatMat2, glm::vec3(0.85f));
        drawBoat(shaderProgram, cubeMesh, sphereMesh, boatMat2);

        // Boat 3 (Center, far horizon)
        glm::mat4 boatMat3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.02f, 30.0f));
        boatMat3 = glm::rotate(boatMat3, glm::radians(8.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        boatMat3 = glm::scale(boatMat3, glm::vec3(0.65f));
        drawBoat(shaderProgram, cubeMesh, sphereMesh, boatMat3);

        // Swap Buffers and Poll Events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. Resource Cleanup
    cubeMesh.cleanup();
    sphereMesh.cleanup();
    canopyGoreMesh.cleanup();
    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// Input Handling
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle Wireframe
    static bool zKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !zKeyPressed)
    {
        g_wireframe = !g_wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, g_wireframe ? GL_LINE : GL_FILL);
        zKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
    {
        zKeyPressed = false;
    }

    // Adjust Robot Head Yaw live
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        g_robotHeadYaw -= 45.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        g_robotHeadYaw += 45.0f * deltaTime;

    // Adjust Umbrella Tilt live
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        g_umbrellaTilt = glm::max(0.0f, g_umbrellaTilt - 20.0f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        g_umbrellaTilt = glm::min(40.0f, g_umbrellaTilt + 20.0f * deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (width > 0 && height > 0)
        glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    camera.handleMouseButton(button, action, xpos, ypos);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.handleMouseMove(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.handleScroll(yoffset);
}
