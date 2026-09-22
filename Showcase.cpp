#include <iostream>
#include <vector>
#include <string>
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
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

// Camera
Camera camera(glm::vec3(0.0f, 1.3f, 0.0f), 7.0f);

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Showcase Controls & States
bool  g_wireframe        = false;
bool  g_autoTurntable    = false;   // Space to toggle auto 360-degree turntable spin
float g_turntableAngle   = 0.0f;

// Interactive Object Demo Variables
float g_robotHeadYaw     = 0.0f;
float g_umbrellaTilt     = 14.0f;

// Definition of Objects in the Showcase Row
struct ShowcaseItem
{
    std::string name;
    float posX;
    float targetY;
    float camDist;
    float camPitch;
    float camYaw;
};

const std::vector<ShowcaseItem> g_showcaseItems = {
    { "1. Palm Tree",              -18.0f, 3.2f, 10.5f, 14.0f, 25.0f },
    { "2. Lifeguard Watch Tower",   -12.0f, 2.2f,  8.0f, 16.0f, 20.0f },
    { "3. Beach Sun Lounger",        -6.0f, 0.8f,  5.0f, 18.0f, 30.0f },
    { "4. Articulated Robot",         0.0f, 1.2f,  4.8f, 16.0f, 20.0f },
    { "5. Beach Umbrella",           +6.0f, 1.8f,  6.5f, 16.0f, 25.0f },
    { "6. Sailboat",                +12.0f, 1.8f,  7.0f, 14.0f, 25.0f },
    { "7. Coastal Rock Formation",  +18.0f, 1.0f,  6.0f, 18.0f, 25.0f }
};

int g_currentIndex = 3; // Start focused on Robot

// Callback Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void focusShowcaseItem(int index, GLFWwindow* window = nullptr)
{
    if (index >= 0 && index < (int)g_showcaseItems.size())
    {
        g_currentIndex = index;
        const auto& item = g_showcaseItems[index];
        camera.desiredTarget = glm::vec3(item.posX, item.targetY, 0.0f);
        camera.desiredDistance = item.camDist;
        camera.desiredYaw = item.camYaw;
        camera.desiredPitch = item.camPitch;
        camera.isTransitioning = true;

        std::cout << "\n>>> [Showcase Focused] " << item.name
                  << " (X = " << item.posX << ") <<<\n";

        if (window)
        {
            std::string title = "3D Object Showcase Gallery - " + item.name;
            glfwSetWindowTitle(window, title.c_str());
        }
    }
}

void focusOverview(GLFWwindow* window = nullptr)
{
    g_currentIndex = -1;
    camera.desiredTarget = glm::vec3(0.0f, 2.0f, 0.0f);
    camera.desiredDistance = 32.0f;
    camera.desiredYaw = 10.0f;
    camera.desiredPitch = 20.0f;
    camera.isTransitioning = true;

    std::cout << "\n>>> [Showcase Focused] Full Row Gallery Overview <<<\n";
    if (window)
    {
        glfwSetWindowTitle(window, "3D Object Showcase Gallery - Full Row Overview [0 / O]");
    }
}



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
        "3D Object Showcase Gallery (In-a-Row Exhibition) - CSE 4102",
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

    std::cout << "===============================================================\n";
    std::cout << "     3D OBJECT MODELING SHOWCASE GALLERY (CSE 4102 LAB)       \n";
    std::cout << "   Student: Hassan Mohammed Naquibul Hoque | Roll: 2107077     \n";
    std::cout << "===============================================================\n";
    std::cout << "  Showcase Alignment (Objects arranged neatly in a row):\n";
    std::cout << "    [1] : 🌴 Palm Tree (Curved trunk, leaf scars, coconuts, fronds)\n";
    std::cout << "    [2] : 🏖️ Lifeguard Watch Tower (Stilts, ladder, roof, lifebuoy)\n";
    std::cout << "    [3] : 🪑 Beach Sun Lounger (Slats, reclining back, armrests)\n";
    std::cout << "    [4] : 🤖 Articulated Robot (Humanoid joints, screen, visor)\n";
    std::cout << "    [5] : ⛱️ Beach Umbrella (Striped canopy gores, tilt, pole)\n";
    std::cout << "    [6] : ⛵ Sailboat (Displacement hull, keel, mast, sails)\n";
    std::cout << "    [7] : 🪨 Coastal Rocks (Composite organic rounded boulders)\n";
    std::cout << "    [0] / [O]   : Full-Row Gallery Overview\n";
    std::cout << "    [TAB] / [->]: Next object in row\n";
    std::cout << "    [<-]        : Previous object in row\n";
    std::cout << "    [SPACE]     : Toggle 360-Degree Turntable Auto-Rotation!\n";
    std::cout << "    [Z]         : Toggle Wireframe mesh view\n";
    std::cout << "    [Mouse]     : Left-drag to Orbit, Right-drag to Pan, Scroll to Zoom\n";
    std::cout << "===============================================================\n";

    // 4. Build and Compile Shader Program
    Shader shaderProgram("default.vert", "default.frag");

    // 5. Instantiate Custom Procedural Base Primitives
    CubeMesh cubeMesh;
    SphereMesh sphereMesh(24, 32, 0.5f);
    CanopyGoreMesh canopyGoreMesh;

    // Studio Lighting Settings
    glm::vec3 sunPosition(10.0f, 20.0f, 22.0f);
    glm::vec3 sunLightColor(1.0f, 0.98f, 0.92f);

    // Initial camera focus on center robot
    focusShowcaseItem(3, window);

    // 6. Main Render Loop
    while (!glfwWindowShouldClose(window))
    {
        // Timing
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Turntable update
        if (g_autoTurntable)
        {
            g_turntableAngle += 30.0f * deltaTime;
            if (g_turntableAngle > 360.0f) g_turntableAngle -= 360.0f;
        }

        // Process Input & Camera
        processInput(window);
        camera.handleKeyboard(window, deltaTime);
        camera.update(deltaTime);

        // Clear Color (Clean studio background)
        glClearColor(0.20f, 0.22f, 0.26f, 1.0f);
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

        glm::vec3 lightDir = glm::normalize(sunPosition);
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
        glUniform3fv(lightColLoc, 1, glm::value_ptr(sunLightColor));
        glUniform1f(ambLoc, 0.38f);

        // ====================================================================
        // EXHIBITION FLOOR
        // ====================================================================
        glm::mat4 floorMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.05f, 0.0f));
        floorMat = glm::scale(floorMat, glm::vec3(60.0f, 0.10f, 20.0f));
        drawTransformedCube(shaderProgram, cubeMesh, floorMat, glm::vec3(0.24f, 0.26f, 0.30f), 16.0f, 0.10f);

        // ====================================================================
        // RENDER OBJECTS IN A ROW (Flat Clean Ground)
        // ====================================================================
        float groundY = 0.0f;

        // --------------------------------------------------------------------
        // OBJECT 1: PALM TREE (X = -18.0)
        // --------------------------------------------------------------------
        {
            float posX = -18.0f;
            glm::mat4 palmMat = glm::translate(glm::mat4(1.0f), glm::vec3(posX, groundY, 0.0f));
            if (g_autoTurntable)
                palmMat = glm::rotate(palmMat, glm::radians(g_turntableAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            else
                palmMat = glm::rotate(palmMat, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            drawPalmTree(shaderProgram, cubeMesh, sphereMesh, palmMat, 1.8f);
        }

        // --------------------------------------------------------------------
        // OBJECT 2: LIFEGUARD WATCH TOWER (X = -12.0)
        // --------------------------------------------------------------------
        {
            float posX = -12.0f;
            glm::mat4 towerMat = glm::translate(glm::mat4(1.0f), glm::vec3(posX, groundY, 0.0f));
            if (g_autoTurntable)
                towerMat = glm::rotate(towerMat, glm::radians(g_turntableAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            else
                towerMat = glm::rotate(towerMat, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            drawLifeguardTower(shaderProgram, cubeMesh, sphereMesh, towerMat);
        }

        // --------------------------------------------------------------------
        // OBJECT 3: BEACH SUN LOUNGER (X = -6.0)
        // --------------------------------------------------------------------
        {
            float posX = -6.0f;
            glm::mat4 chairMat = glm::translate(glm::mat4(1.0f), glm::vec3(posX, groundY, 0.0f));
            if (g_autoTurntable)
                chairMat = glm::rotate(chairMat, glm::radians(g_turntableAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            else
                chairMat = glm::rotate(chairMat, glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            drawBeachChair(shaderProgram, cubeMesh, chairMat);
        }

        // --------------------------------------------------------------------
        // OBJECT 4: ARTICULATED ROBOT (X = 0.0, Standing Heroic Display Pose)
        // --------------------------------------------------------------------
        {
            float posX = 0.0f;
            // Robot pelvis origin is at center of body; standing feet rest directly on ground at groundY
            glm::mat4 robotMat = glm::translate(glm::mat4(1.0f), glm::vec3(posX, groundY + 1.25f, 0.0f));
            if (g_autoTurntable)
                robotMat = glm::rotate(robotMat, glm::radians(g_turntableAngle), glm::vec3(0.0f, 1.0f, 0.0f));

            // Standing pose: straight legs (legBend = 0), right arm waving (35 deg), left arm relaxed (-15 deg)
            drawRobot(shaderProgram, cubeMesh, sphereMesh,
                robotMat,
                g_robotHeadYaw,  // headYaw
                0.0f,            // headPitch
                -15.0f,          // leftArmPitch
                35.0f,           // rightArmPitch (friendly wave)
                0.0f);           // legBendAngle (standing upright)
        }

        // --------------------------------------------------------------------
        // OBJECT 5: BEACH UMBRELLA (X = +6.0)
        // --------------------------------------------------------------------
        {
            float posX = +6.0f;
            glm::mat4 umbrellaMat = glm::translate(glm::mat4(1.0f), glm::vec3(posX, groundY, 0.0f));
            if (g_autoTurntable)
                umbrellaMat = glm::rotate(umbrellaMat, glm::radians(g_turntableAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            drawUmbrella(shaderProgram, cubeMesh, sphereMesh, canopyGoreMesh,
                umbrellaMat, g_umbrellaTilt, 3.4f, 1.0f);
        }

        // --------------------------------------------------------------------
        // OBJECT 6: SAILBOAT (X = +12.0)
        // --------------------------------------------------------------------
        {
            float posX = +12.0f;
            // Boat origin has keel at Y = -0.18m; raise so keel rests directly on floor
            glm::mat4 boatMat = glm::translate(glm::mat4(1.0f), glm::vec3(posX, groundY + 0.18f, 0.0f));
            if (g_autoTurntable)
                boatMat = glm::rotate(boatMat, glm::radians(g_turntableAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            else
                boatMat = glm::rotate(boatMat, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            drawBoat(shaderProgram, cubeMesh, sphereMesh, boatMat);
        }

        // --------------------------------------------------------------------
        // OBJECT 7: COASTAL ROCK FORMATION (X = +18.0)
        // --------------------------------------------------------------------
        {
            float posX = +18.0f;
            glm::mat4 rockMat = glm::translate(glm::mat4(1.0f), glm::vec3(posX, groundY, 0.0f));
            if (g_autoTurntable)
                rockMat = glm::rotate(rockMat, glm::radians(g_turntableAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            drawRockCluster(shaderProgram, cubeMesh, sphereMesh, rockMat);
        }

        // Swap Buffers and Poll Events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 7. Cleanup
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
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE) zKeyPressed = false;

    // Toggle Auto-Turntable (Space)
    static bool spacePressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed)
    {
        g_autoTurntable = !g_autoTurntable;
        std::cout << "[Turntable Rotation] " << (g_autoTurntable ? "ENABLED (360 Auto-Spin)" : "PAUSED") << std::endl;
        spacePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) spacePressed = false;

    // Direct Object Selection (1 through 7)
    for (int k = 1; k <= 7; ++k)
    {
        int key = GLFW_KEY_0 + k;
        if (glfwGetKey(window, key) == GLFW_PRESS)
        {
            focusShowcaseItem(k - 1, window);
        }
    }

    // 0 or O: Full Overview
    static bool oPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) && !oPressed)
    {
        focusOverview(window);
        oPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
        oPressed = false;

    // TAB or Right Arrow: Next Object in Row
    static bool nextPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) && !nextPressed)
    {
        int next = (g_currentIndex + 1) % (int)g_showcaseItems.size();
        focusShowcaseItem(next, window);
        nextPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
        nextPressed = false;

    // Left Arrow: Previous Object in Row
    static bool prevPressed = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !prevPressed)
    {
        int prev = (g_currentIndex - 1 + (int)g_showcaseItems.size()) % (int)g_showcaseItems.size();
        focusShowcaseItem(prev, window);
        prevPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
        prevPressed = false;

    // Live demonstration adjustments
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        g_robotHeadYaw -= 45.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        g_robotHeadYaw += 45.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        g_umbrellaTilt = glm::max(0.0f, g_umbrellaTilt - 20.0f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
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
