#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderClass.h"
#include "MeshPrimitives.h"
#include "TextureManager.h"
#include "Skybox.h"
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

// ============================================================================
// PROPOSAL REQUIREMENTS STATE VARIABLES
// ============================================================================
// 1. Day-Cycle Simulation (Sunrise to Sunset transition)
float g_dayTime       = 7.0f;    // 24-hour clock (e.g. 7.0 = 7:00 AM Sunrise)
float g_daySpeed      = 0.35f;   // Hours per second
bool  g_dayPaused     = false;

// 2. Texture Mapping & Visual Overrides
bool  g_useTextures   = true;    // [X] to toggle procedural texture mapping
bool  g_renderShadows = true;    // [G] to toggle dynamic projected shadows
int   g_shadingMode   = 0;       // [M] to cycle: 0=Auto (Proposal), 1=Flat, 2=Gouraud, 3=Phong
bool  g_wireframe     = false;   // [Z] to toggle wireframe mode

// 3. Interactive Object Modeling
float g_robotHeadYaw     = 15.0f;
float g_robotLeftArm     = -20.0f;
float g_robotRightArm    = 30.0f;
float g_umbrellaTilt     = 14.0f;
float g_palmTrunkCurve   = 3.2f;

// 4. Interactive Robot Motorboat Driving Simulation
struct DrivableBoatState
{
    glm::vec3 pos            = glm::vec3(8.0f, 0.0f, 78.0f); // Start offshore south of the island
    float heading            = -22.0f; // Compass heading (deg); 0 = +Z
    float speed              = 0.0f;   // Longitudinal hull speed (m/s)
    float maxSpeed           = 12.0f;  // ~23 knots (realistic for 20ft outboard)
    float maxReverseSpeed    = -2.8f;  // ~5.5 knots reverse
    float acceleration       = 4.5f;   // Engine thrust-to-mass ratio
    float brakeDecel         = 6.0f;   // Reverse thrust braking
    float rudderAngle        = 0.0f;   // Outboard pivot angle (-35 to +35 deg)
    float rudderSpeed        = 55.0f;  // deg/sec helm response
    float throttleInput      = 0.0f;   // Raw throttle input (-1 to 1)
    float propellerAngle     = 0.0f;   // Spinning prop visual angle

    // Smooth angular state (lerped for visual stability)
    float pitch              = 0.0f;   // Current smoothed pitch (deg)
    float roll               = 0.0f;   // Current smoothed roll (deg)
    float targetPitch        = 0.0f;   // Target pitch from physics
    float targetRoll         = 0.0f;   // Target roll from physics
    float yawVelocity        = 0.0f;   // Angular yaw rate (deg/s) for momentum

    // Hull waterline reference height (half-draft)
    float draftOffset        = 0.15f;  // How deep the hull sits in water
};

DrivableBoatState g_boatState;
bool g_boatPilotMode = false; // [F] / [0] to toggle interactive 3rd-person Pilot Mode

struct BeachATVState
{
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 43.0f);
    float heading = 90.0f;
    float speed = 0.0f;
    float steering = 0.0f;
    float wheelSpin = 0.0f;
};

BeachATVState g_atvState;
bool g_atvDriveMode = false; // [H] toggles beach-ring ATV driving

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
        "Beach Day: A Full Sunrise-to-Sunset Coastal Scene in OpenGL - CSE 4102",
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "=======================================================================\n";
    std::cout << "  BEACH DAY: FULL SUNRISE-TO-SUNSET COASTAL SCENE (CSE 4102 LABORATORY)\n";
    std::cout << "  Student: Hassan Mohammed Naquibul Hoque | Roll: 2107077\n";
    std::cout << "=======================================================================\n";
    std::cout << "  Day-Cycle & Lighting Controls:\n";
    std::cout << "    [SPACE]            : Pause / Resume day cycle progression\n";
    std::cout << "    [ [ ] / [ ] ]      : Slow down / Speed up day-cycle transition\n";
    std::cout << "    [J] / [F5]         : Instant jump to SUNRISE (06:30)\n";
    std::cout << "    [K] / [F6]         : Instant jump to MIDDAY NOON (12:00)\n";
    std::cout << "    [L] / [F7]         : Instant jump to SUNSET (18:15)\n";
    std::cout << "    [N] / [F8]         : Instant jump to TWILIGHT / NIGHT (21:30)\n";
    std::cout << "\n";
    std::cout << "  Shading & Graphics Techniques:\n";
    std::cout << "    [M]                : Cycle Shading Mode (Auto Proposal / Flat / Gouraud / Phong)\n";
    std::cout << "    [V]                : Cycle Camera Projection (Perspective / Ortho Top / Ortho Side)\n";
    std::cout << "    [X]                : Toggle Texture Mapping ON / OFF (Sand, Water, Sail, Palms)\n";
    std::cout << "    [G]                : Toggle Dynamic Projected Shadows ON / OFF\n";
    std::cout << "    [Z]                : Toggle Wireframe Mode\n";
    std::cout << "\n";
    std::cout << "  Quick Camera Focus Presets:\n";
    std::cout << "    [B] / [5]          : Focus on ROBOT MOTORBOAT (Sleek hull, helm, robot driver)\n";
    std::cout << "    [T] / [6]          : Focus on LIFEGUARD TOWER (Stilts, ladder, roof)\n";
    std::cout << "    [C] / [7]          : Focus on CENTER ROBOT & UMBRELLA\n";
    std::cout << "    [P] / [8]          : Focus on PALM TREES & COASTAL ROCKS\n";
    std::cout << "    [O] / [R]          : Full Scenic Beach Panorama Overview\n";
    std::cout << "    [TAB]              : Tour / Cycle through every object one-by-one\n";
    std::cout << "\n";
    std::cout << "  Interactive Robot Boat Pilot Mode:\n";
    std::cout << "    [F] / [0]          : TOGGLE BOAT PILOT MODE (3rd-Person Chase Cam!)\n";
    std::cout << "    [UP] / [W]         : Throttle Forward (Accelerate through ocean waves)\n";
    std::cout << "    [DOWN] / [S]       : Throttle Reverse / Brake\n";
    std::cout << "    [LEFT] / [A]       : Steer Port (Left) - Robot steers wheel & glances left\n";
    std::cout << "    [RIGHT] / [D]      : Steer Starboard (Right) - Robot steers wheel & glances right\n";
    std::cout << "    [H]                : Toggle BEACH ATV DRIVE MODE (WASD beach-ring tour)\n";
    std::cout << "\n";
    std::cout << "  Beach Resort Interactive Parameters:\n";
    std::cout << "    [1 / 2]            : Turn Beach Resort Robot Head Left / Right\n";
    std::cout << "    [3 / 4]            : Adjust Umbrella Tilt Angle\n";
    std::cout << "=======================================================================\n";

    // 4. Build and Compile Shaders
    Shader shaderProgram("default.vert", "default.frag");
    Shader waterShader("water.vert", "water.frag");

    // 5. Instantiate Skybox & Procedural Texture Manager
    Skybox skybox;
    TextureManager texManager;
    texManager.init();

    // 6. Base Geometric Primitives
    CubeMesh cubeMesh;
    SphereMesh sphereMesh(24, 32, 0.5f);
    CanopyGoreMesh canopyGoreMesh;
    WaterGridMesh waterMesh;
    SlopingSeabedMesh seabedMesh;
    WaterDepthSkirtMesh waterSkirtMesh;
    IslandTerrainMesh islandTerrainMesh;
    FrustumMesh wheelCylinderMesh(0.5f, 0.5f, 32);
    FrustumMesh lighthouseTowerMesh(0.5f, 0.34f, 40);
    FrustumMesh lighthouseRoofMesh(0.55f, 0.0f, 40);
    FrustumMesh lighthouseBeamMesh(0.5f, 0.0f, 48, false);

    // Start on the intended establishing composition instead of a close crop.
    camera.setPreset(VIEW_OVERVIEW, window);
    camera.update(1.0f);

    // 7. Main Render Loop
    while (!glfwWindowShouldClose(window))
    {
        // Timing
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process Input & Camera
        processInput(window);
        if (g_boatPilotMode)
        {
            // Dynamic Third-Person Chase Camera behind the Robot's Motorboat
            camera.isTransitioning = false;
            camera.target = glm::mix(camera.target, g_boatState.pos + glm::vec3(0.0f, 1.15f, 0.0f), glm::clamp(deltaTime * 8.0f, 0.0f, 1.0f));
            float desiredYaw = 180.0f - g_boatState.heading;
            float yawDiff = desiredYaw - camera.yaw;
            while (yawDiff < -180.0f) yawDiff += 360.0f;
            while (yawDiff > 180.0f) yawDiff -= 360.0f;
            camera.yaw += yawDiff * glm::clamp(deltaTime * 4.5f, 0.0f, 1.0f);
            camera.pitch = glm::mix(camera.pitch, 15.0f, glm::clamp(deltaTime * 4.0f, 0.0f, 1.0f));
            camera.distance = glm::mix(camera.distance, 8.2f, glm::clamp(deltaTime * 4.0f, 0.0f, 1.0f));
        }
        else if (g_atvDriveMode)
        {
            camera.isTransitioning = false;
            camera.target = glm::mix(camera.target, g_atvState.pos + glm::vec3(0.0f, 1.1f, 0.0f), glm::clamp(deltaTime * 8.0f, 0.0f, 1.0f));
            float desiredYaw = 180.0f - g_atvState.heading;
            float yawDiff = desiredYaw - camera.yaw;
            while (yawDiff < -180.0f) yawDiff += 360.0f;
            while (yawDiff > 180.0f) yawDiff -= 360.0f;
            camera.yaw += yawDiff * glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f);
            camera.pitch = glm::mix(camera.pitch, 17.0f, glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f));
            camera.distance = glm::mix(camera.distance, 7.5f, glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f));
        }
        else
        {
            camera.handleKeyboard(window, deltaTime);
            if (camera.currentPresetIndex == (int)VIEW_BOAT && !camera.isTransitioning)
            {
                camera.target = glm::mix(camera.target, g_boatState.pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::clamp(deltaTime * 6.0f, 0.0f, 1.0f));
            }
        }
        camera.update(deltaTime);

        // Advance Day Cycle
        if (!g_dayPaused)
        {
            g_dayTime += deltaTime * g_daySpeed;
            if (g_dayTime >= 24.0f) g_dayTime -= 24.0f;
            if (g_dayTime < 0.0f)   g_dayTime += 24.0f;
        }

        // ====================================================================
        // SUN ARC & LIGHTING CALCULATIONS (TRANSLATION, ROTATION, SCALING)
        // ====================================================================
        // The sun arcs from East (+X, +Z) at dawn up overhead (+Y) at midday to West (-X, +Z) at dusk.
        glm::vec3 sunPosition;
        glm::vec3 sunLightColor;
        glm::vec3 sunVisualColor;
        float ambientStrength = 0.35f;
        float sunElevation = 0.0f;

        // Daytime arc (5.0 AM to 19.5 PM)
        if (g_dayTime >= 5.0f && g_dayTime <= 19.5f)
        {
            float dayProgress = (g_dayTime - 5.0f) / 14.5f; // 0.0 (sunrise) to 1.0 (sunset)
            float sunAngle = dayProgress * 3.14159265f;      // 0 to PI

            // Positioning the celestial sun at true vast astronomical distance (R = 750m)
            float celestialRadius = 750.0f;
            float sunX = -cosf(sunAngle) * celestialRadius;
            float sunY = sinf(sunAngle) * 480.0f + 15.0f;
            float sunZ = 380.0f - sinf(sunAngle) * 120.0f; // Far seaward horizon towards +Z
            sunPosition = glm::vec3(sunX, sunY, sunZ);
            sunElevation = sunY;

            // Sunlight Color and Intensity Shift throughout the Day
            if (g_dayTime < 8.0f)
            {
                // Sunrise: warm golden/rose tones
                float t = (g_dayTime - 5.0f) / 3.0f;
                sunLightColor  = glm::mix(glm::vec3(1.0f, 0.45f, 0.20f), glm::vec3(1.0f, 0.95f, 0.85f), t);
                sunVisualColor = glm::mix(glm::vec3(1.0f, 0.50f, 0.25f), glm::vec3(1.0f, 0.95f, 0.45f), t);
                ambientStrength = glm::mix(0.22f, 0.38f, t);
            }
            else if (g_dayTime <= 16.5f)
            {
                // Midday: bright midday white sunlight
                sunLightColor  = glm::vec3(1.0f, 0.98f, 0.92f);
                sunVisualColor = glm::vec3(1.0f, 0.96f, 0.55f);
                ambientStrength = 0.42f;
            }
            else
            {
                // Sunset: warm crimson/orange hues
                float t = (g_dayTime - 16.5f) / 3.0f;
                sunLightColor  = glm::mix(glm::vec3(1.0f, 0.98f, 0.92f), glm::vec3(1.0f, 0.38f, 0.15f), t);
                sunVisualColor = glm::mix(glm::vec3(1.0f, 0.96f, 0.55f), glm::vec3(1.0f, 0.42f, 0.20f), t);
                ambientStrength = glm::mix(0.42f, 0.20f, t);
            }
        }
        else
        {
            // Night / Twilight (cool blue ambient moonlight)
            sunPosition = glm::vec3(0.0f, -150.0f, 350.0f);
            sunLightColor = glm::vec3(0.18f, 0.24f, 0.45f);
            sunVisualColor = glm::vec3(0.25f, 0.35f, 0.55f);
            ambientStrength = 0.15f;
            sunElevation = -150.0f;
        }

        // Atmospheric sun disc scaling
        float horizonFactor = glm::clamp(sunPosition.y / 480.0f, 0.0f, 1.0f);
        float discScale = 1.0f + 0.65f * glm::pow(1.0f - horizonFactor, 2.0f);
        float currentSunRadius = 12.0f * discScale;

        // View and Projection Matrices
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT);
        glm::vec3 camPos = camera.getPosition();
        glm::vec3 lightDir = glm::normalize(sunPosition);

        // ====================================================================
        // RENDER 1: DYNAMIC SUNRISE-TO-SUNSET SKYBOX
        // ====================================================================
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox.draw(view, proj, g_dayTime, lightDir);

        // ====================================================================
        // SHADER UNIFORMS & GLOBAL SETTINGS
        // ====================================================================
        shaderProgram.Activate();

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "viewPos"), 1, glm::value_ptr(camPos));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "lightDir"), 1, glm::value_ptr(lightDir));
        glUniform3fv(glGetUniformLocation(shaderProgram.ID, "lightColor"), 1, glm::value_ptr(sunLightColor));
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "ambientStrength"), ambientStrength);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "isShadowPass"), 0);
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "objectAlpha"), 1.0f);
        glUniform1i(glGetUniformLocation(shaderProgram.ID, "beaconBeamPass"), 0);

        // Apply Global Shading Mode Override if active
        if (g_shadingMode > 0)
        {
            // 1: All Flat, 2: All Gouraud, 3: All Phong
            glUniform1i(glGetUniformLocation(shaderProgram.ID, "shadingMode"), g_shadingMode - 1);
        }

        // ====================================================================
        // ENVIRONMENTAL SIMULATION DYNAMICS
        // ====================================================================
        // A. Shifting Tide Line: water edge gently shifts along shore
        float tideOffset = sinf(currentFrame * 0.75f) * 1.8f + cosf(currentFrame * 0.35f) * 0.6f;
        glm::vec3 seaCenter(0.0f, -0.12f, 22.0f - tideOffset * 0.5f);
        float seaDepth = 40.0f + tideOffset;

        // B. Wind Swaying: palm leaves & umbrella gently sway in sea breeze
        float windSway = sinf(currentFrame * 2.2f) * 1.6f + cosf(currentFrame * 1.1f) * 0.8f;
        float dynamicUmbrellaTilt = g_umbrellaTilt + sinf(currentFrame * 1.6f) * 1.8f;

        // C. DRIVABLE BEACH ATV - constrained to the continuous sand ring.
        if (g_atvDriveMode)
        {
            float throttle = 0.0f;
            float steer = 0.0f;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) throttle += 1.0f;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) throttle -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) steer += 1.0f;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) steer -= 1.0f;

            g_atvState.speed += throttle * 7.0f * deltaTime;
            if (throttle == 0.0f)
                g_atvState.speed *= powf(0.16f, deltaTime);
            g_atvState.speed = glm::clamp(g_atvState.speed, -3.0f, 10.0f);
            g_atvState.steering = glm::mix(g_atvState.steering, steer * 28.0f, glm::clamp(deltaTime * 7.0f, 0.0f, 1.0f));

            float speedTurn = glm::clamp(fabsf(g_atvState.speed) / 4.0f, 0.0f, 1.0f);
            g_atvState.heading += g_atvState.steering * speedTurn * (g_atvState.speed >= 0.0f ? 1.0f : -1.0f) * deltaTime;
            float headingRad = glm::radians(g_atvState.heading);
            glm::vec3 forward(sinf(headingRad), 0.0f, cosf(headingRad));
            glm::vec3 candidate = g_atvState.pos + forward * g_atvState.speed * deltaTime;
            float beachDistance = getIslandSignedDistance(candidate.x, candidate.z);
            if (beachDistance > 3.0f && beachDistance < 25.0f)
                g_atvState.pos = candidate;
            else
                g_atvState.speed *= -0.12f;
            g_atvState.pos.y = getCoastalTerrainElevation(g_atvState.pos.x, g_atvState.pos.z);
            g_atvState.wheelSpin += g_atvState.speed * deltaTime * 95.0f;
        }

        // ================================================================
        // D. REALISTIC MOTORBOAT PHYSICS SIMULATION
        // Multi-point wave buoyancy, hull drag model, angular momentum,
        // wave-resistance, planing dynamics, and smoothed orientation.
        // ================================================================
        float throttleInput = 0.0f;
        float steerInput = 0.0f;

        if (g_boatPilotMode)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                throttleInput += 1.0f;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                throttleInput -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                steerInput -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                steerInput += 1.0f;
        }
        else
        {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                throttleInput += 1.0f;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                throttleInput -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                steerInput -= 1.0f;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                steerInput += 1.0f;
        }
        g_boatState.throttleInput = throttleInput;

        // --- Rudder Steering with Momentum ---
        if (steerInput != 0.0f)
        {
            g_boatState.rudderAngle += steerInput * g_boatState.rudderSpeed * deltaTime;
            g_boatState.rudderAngle = glm::clamp(g_boatState.rudderAngle, -35.0f, 35.0f);
        }
        else
        {
            // Hydraulic self-centering with speed-dependent return rate
            float returnRate = 2.5f + fabs(g_boatState.speed) * 0.3f;
            g_boatState.rudderAngle = glm::mix(g_boatState.rudderAngle, 0.0f, glm::clamp(deltaTime * returnRate, 0.0f, 1.0f));
        }

        // --- Hull Drag Model (skin friction + form drag + wave-making resistance) ---
        float absSpeed = fabs(g_boatState.speed);
        float speedFrac = absSpeed / g_boatState.maxSpeed;
        // Skin friction: linear at low speed
        float skinFriction = 0.35f * absSpeed;
        // Form drag: quadratic at higher speeds (hull pushing water aside)
        float formDrag = 0.08f * absSpeed * absSpeed;
        // Wave-making resistance: peaks near hull speed, drops when planing
        float hullSpeedKmh = 2.43f * sqrtf(6.0f); // Hull speed for ~6m waterline
        float waveResistance = 0.15f * expf(-2.0f * powf(absSpeed - hullSpeedKmh * 0.5f, 2.0f) / 9.0f);
        float totalDrag = (skinFriction + formDrag + waveResistance) * deltaTime;

        // --- Engine Force & Speed Integration ---
        if (throttleInput > 0.0f)
        {
            // Thrust decreases as speed approaches max (propeller efficiency curve)
            float thrustEfficiency = 1.0f - powf(speedFrac, 2.2f);
            g_boatState.speed += g_boatState.acceleration * throttleInput * thrustEfficiency * deltaTime;
            g_boatState.speed = glm::min(g_boatState.speed, g_boatState.maxSpeed);
        }
        else if (throttleInput < 0.0f)
        {
            if (g_boatState.speed > 0.3f)
                g_boatState.speed -= g_boatState.brakeDecel * deltaTime;
            else
            {
                g_boatState.speed += throttleInput * 3.0f * deltaTime;
                g_boatState.speed = glm::max(g_boatState.speed, g_boatState.maxReverseSpeed);
            }
        }

        // Apply drag deceleration
        if (g_boatState.speed > 0.0f)
            g_boatState.speed = glm::max(0.0f, g_boatState.speed - totalDrag);
        else if (g_boatState.speed < 0.0f)
            g_boatState.speed = glm::min(0.0f, g_boatState.speed + totalDrag * 0.6f);
        if (fabs(g_boatState.speed) < 0.015f && throttleInput == 0.0f) g_boatState.speed = 0.0f;

        // --- Heading / Yaw with Angular Momentum ---
        // Turn force: rudder effect scales with water flow over the outboard
        float rudderNorm = g_boatState.rudderAngle / 35.0f;
        float flowSpeed = g_boatState.speed;
        // Even at rest with throttle, prop wash over rudder gives some steerage
        if (throttleInput != 0.0f && fabs(flowSpeed) < 1.5f)
            flowSpeed = (throttleInput > 0.0f) ? 1.5f : -1.5f;

        float turnForce = -rudderNorm * (flowSpeed / g_boatState.maxSpeed) * 85.0f;
        // Apply angular momentum: yaw velocity smoothly integrates turn force
        g_boatState.yawVelocity += (turnForce - g_boatState.yawVelocity * 3.2f) * deltaTime;
        g_boatState.heading += g_boatState.yawVelocity * deltaTime;

        // --- Propeller Visual Spin ---
        float propRPM = absSpeed * 18.0f + (throttleInput != 0.0f ? fabs(throttleInput) * 25.0f : 1.5f);
        g_boatState.propellerAngle += propRPM * 360.0f * deltaTime;

        // --- Position Integration along Heading Vector ---
        float radHeading = glm::radians(g_boatState.heading);
        glm::vec3 forwardDir(sinf(radHeading), 0.0f, cosf(radHeading));
        glm::vec3 rightDir(cosf(radHeading), 0.0f, -sinf(radHeading));
        g_boatState.pos += forwardDir * g_boatState.speed * deltaTime;

        // Navigation boundaries (coastline and open sea limits)
        if (g_boatState.pos.z < 5.5f)
        {
            g_boatState.pos.z = 5.5f;
            float dotFwd = glm::dot(forwardDir, glm::vec3(0, 0, -1));
            if ((g_boatState.speed > 0.0f && dotFwd > 0.3f) || (g_boatState.speed < 0.0f && dotFwd < -0.3f))
            {
                g_boatState.speed *= 0.5f; // Soft collision, bleed speed
            }
        }
        if (g_boatState.pos.z > 220.0f) g_boatState.pos.z = 220.0f;
        g_boatState.pos.x = glm::clamp(g_boatState.pos.x, -160.0f, 160.0f);

        // --- Multi-Point Gerstner Wave Buoyancy (4 hull sample points) ---
        // Sample wave height at bow, stern, port-midship, and starboard-midship
        float hullLen = 2.0f;  // Half-length bow-to-stern
        float hullBeam = 0.65f; // Half-beam width

        glm::vec3 bowWorldPos  = g_boatState.pos + forwardDir * hullLen;
        glm::vec3 sternWorldPos = g_boatState.pos - forwardDir * hullLen;
        glm::vec3 portWorldPos  = g_boatState.pos - rightDir * hullBeam;
        glm::vec3 stbdWorldPos  = g_boatState.pos + rightDir * hullBeam;

        glm::vec3 bowWP, bowWN, sternWP, sternWN, portWP, portWN, stbdWP, stbdWN, centerWP, centerWN;
        getGerstnerWaveAt(bowWorldPos.x, bowWorldPos.z, currentFrame, bowWP, bowWN);
        getGerstnerWaveAt(sternWorldPos.x, sternWorldPos.z, currentFrame, sternWP, sternWN);
        getGerstnerWaveAt(portWorldPos.x, portWorldPos.z, currentFrame, portWP, portWN);
        getGerstnerWaveAt(stbdWorldPos.x, stbdWorldPos.z, currentFrame, stbdWP, stbdWN);
        getGerstnerWaveAt(g_boatState.pos.x, g_boatState.pos.z, currentFrame, centerWP, centerWN);

        // Average height from all 5 sample points (weighted: center heavier)
        float avgY = (centerWP.y * 3.0f + bowWP.y + sternWP.y + portWP.y + stbdWP.y) / 7.0f;
        g_boatState.pos.y = avgY + g_boatState.draftOffset;

        // Pitch from bow-stern height differential
        float bowSternDiff = bowWP.y - sternWP.y;
        float wavePitch = glm::degrees(atanf(bowSternDiff / (hullLen * 2.0f)));

        // Roll from port-starboard height differential
        float portStbdDiff = stbdWP.y - portWP.y;
        float waveRoll = glm::degrees(atanf(portStbdDiff / (hullBeam * 2.0f)));

        // --- Wave Resistance: Opposing wave slopes slow the boat ---
        float waveSlopeAgainstBow = -glm::dot(glm::vec3(centerWN.x, 0.0f, centerWN.z), forwardDir);
        float waveImpact = glm::clamp(waveSlopeAgainstBow * absSpeed * 0.08f, -0.3f, 0.3f);
        g_boatState.speed -= waveImpact * deltaTime;

        // --- Hydrodynamic Planing Bow Rise ---
        // At displacement speeds: bow level. At planing speeds: bow lifts 2-4 deg
        float planingThreshold = 0.45f; // ~45% of max speed to begin planing
        float planingFactor = glm::smoothstep(planingThreshold, 0.85f, speedFrac);
        float planingPitch = planingFactor * 3.5f;

        // --- Centripetal Turn Roll (bank inwards during turns) ---
        float centripetalRoll = -rudderNorm * speedFrac * 6.0f;

        // --- Speed-based vertical bob dampening (boats at speed skim over chop) ---
        float chopDampen = glm::mix(1.0f, 0.5f, glm::smoothstep(0.3f, 0.8f, speedFrac));

        // Compute target pitch/roll
        g_boatState.targetPitch = wavePitch * chopDampen + planingPitch;
        g_boatState.targetRoll  = waveRoll * chopDampen + centripetalRoll;

        // --- Smooth Angular State (simulates rotational inertia of hull mass) ---
        float angularSmooth = glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f);
        g_boatState.pitch = glm::mix(g_boatState.pitch, g_boatState.targetPitch, angularSmooth);
        g_boatState.roll  = glm::mix(g_boatState.roll, g_boatState.targetRoll, angularSmooth);

        // --- Build Final Boat Transform Matrix ---
        glm::mat4 boatMat1 = glm::translate(glm::mat4(1.0f), g_boatState.pos);
        boatMat1 = glm::rotate(boatMat1, glm::radians(g_boatState.heading), glm::vec3(0.0f, 1.0f, 0.0f));
        boatMat1 = glm::rotate(boatMat1, glm::radians(g_boatState.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        boatMat1 = glm::rotate(boatMat1, glm::radians(g_boatState.roll), glm::vec3(0.0f, 0.0f, 1.0f));
        boatMat1 = glm::scale(boatMat1, glm::vec3(1.65f));

        // Distant Boats with wave buoyancy
        float boat2X = -12.0f + sinf(currentFrame * 0.15f) * 2.0f;
        float boat2Z = 108.0f;
        glm::vec3 boat2WavePos, boat2WaveNorm;
        getGerstnerWaveAt(boat2X, boat2Z, currentFrame, boat2WavePos, boat2WaveNorm);
        float boat2Pitch = glm::degrees(asinf(glm::clamp(-boat2WaveNorm.z, -0.6f, 0.6f)));
        float boat2Roll  = glm::degrees(asinf(glm::clamp(boat2WaveNorm.x, -0.6f, 0.6f)));

        glm::mat4 boatMat2 = glm::translate(glm::mat4(1.0f), boat2WavePos);
        boatMat2 = glm::rotate(boatMat2, glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        boatMat2 = glm::rotate(boatMat2, glm::radians(boat2Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        boatMat2 = glm::rotate(boatMat2, glm::radians(boat2Roll), glm::vec3(0.0f, 0.0f, 1.0f));
        boatMat2 = glm::scale(boatMat2, glm::vec3(1.30f));

        float boat3X = 8.5f + sinf(currentFrame * 0.10f) * 1.5f;
        float boat3Z = 142.0f;
        glm::vec3 boat3WavePos, boat3WaveNorm;
        getGerstnerWaveAt(boat3X, boat3Z, currentFrame, boat3WavePos, boat3WaveNorm);
        float boat3Pitch = glm::degrees(asinf(glm::clamp(-boat3WaveNorm.z, -0.6f, 0.6f)));
        float boat3Roll  = glm::degrees(asinf(glm::clamp(boat3WaveNorm.x, -0.6f, 0.6f)));

        glm::mat4 boatMat3 = glm::translate(glm::mat4(1.0f), boat3WavePos);
        boatMat3 = glm::rotate(boatMat3, glm::radians(8.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        boatMat3 = glm::rotate(boatMat3, glm::radians(boat3Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        boatMat3 = glm::rotate(boatMat3, glm::radians(boat3Roll), glm::vec3(0.0f, 0.0f, 1.0f));
        boatMat3 = glm::scale(boatMat3, glm::vec3(1.15f));

        // Transforms for Main Beach Objects
        float southBeachY = getCoastalTerrainElevation(0.0f, 39.0f);
        glm::mat4 chairMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, southBeachY, 39.0f));
        glm::mat4 robotMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, southBeachY + 0.62f, 39.05f));
        glm::mat4 umbrellaMat1 = glm::translate(glm::mat4(1.0f), glm::vec3(-1.6f, southBeachY, 37.8f));

        struct BeachChairPlacement { float x, z, yaw, scale; };
        const BeachChairPlacement extraChairs[] = {
            { -32.0f, 34.0f,  12.0f, 1.00f }, { -24.0f, 39.0f,   8.0f, 1.05f },
            { -16.0f, 42.0f,   5.0f, 0.96f }, {  -8.0f, 44.0f,   2.0f, 1.02f },
            {   9.0f, 44.0f,  -3.0f, 1.00f }, {  18.0f, 41.0f,  -8.0f, 1.06f },
            {  28.0f, 36.0f, -14.0f, 0.98f }, {  74.0f,  4.0f, -58.0f, 1.03f },
            {  84.0f,-17.0f, -72.0f, 0.96f }, {  91.0f,-39.0f, -84.0f, 1.05f },
            {  94.0f,-63.0f, -90.0f, 1.00f }, {  96.0f,-88.0f, -94.0f, 1.04f },
            {  93.0f,-113.0f,-99.0f, 0.98f }, {  86.0f,-137.0f,-108.0f,1.02f }
        };
        auto makeChairTransform = [](const BeachChairPlacement& placement)
        {
            float y = getCoastalTerrainElevation(placement.x, placement.z);
            glm::mat4 result = glm::translate(glm::mat4(1.0f), glm::vec3(placement.x, y, placement.z));
            result = glm::rotate(result, glm::radians(placement.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
            return glm::scale(result, glm::vec3(placement.scale));
        };

        glm::mat4 chairMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(-7.5f, 0.0f, -1.6f));
        chairMat2 = glm::rotate(chairMat2, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 robotMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(-7.5f, 0.62f, -1.55f));
        robotMat2 = glm::rotate(robotMat2, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 umbrellaMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(-8.6f, 0.0f, -2.4f));

        glm::mat4 chairMat3 = glm::translate(glm::mat4(1.0f), glm::vec3(7.5f, 0.0f, -1.6f));
        chairMat3 = glm::rotate(chairMat3, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 robotMat3 = glm::translate(glm::mat4(1.0f), glm::vec3(7.5f, 0.62f, -1.55f));
        robotMat3 = glm::rotate(robotMat3, glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 umbrellaMat3 = glm::translate(glm::mat4(1.0f), glm::vec3(6.4f, 0.0f, -2.4f));

        glm::mat4 chairMat4 = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, -2.2f));
        chairMat4 = glm::rotate(chairMat4, glm::radians(-25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 umbrellaMat4 = glm::translate(glm::mat4(1.0f), glm::vec3(14.0f, 0.0f, -3.0f));

        glm::mat4 towerMat = glm::translate(glm::mat4(1.0f), glm::vec3(-4.2f, 0.0f, 3.2f));
        towerMat = glm::rotate(towerMat, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 leftPalmMat = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, getCoastalTerrainElevation(-30.0f, 29.0f), 29.0f));
        leftPalmMat = glm::rotate(leftPalmMat, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 backLeftPalm = glm::translate(glm::mat4(1.0f), glm::vec3(-15.0f, getCoastalTerrainElevation(-15.0f, 23.0f), 23.0f));
        backLeftPalm = glm::rotate(backLeftPalm, glm::radians(70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        backLeftPalm = glm::scale(backLeftPalm, glm::vec3(1.10f));

        glm::mat4 backRightPalm = glm::translate(glm::mat4(1.0f), glm::vec3(17.0f, getCoastalTerrainElevation(17.0f, 24.0f), 24.0f));
        backRightPalm = glm::rotate(backRightPalm, glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        backRightPalm = glm::scale(backRightPalm, glm::vec3(1.15f));

        glm::mat4 rightPalmMat = glm::translate(glm::mat4(1.0f), glm::vec3(34.0f, getCoastalTerrainElevation(34.0f, 28.0f), 28.0f));
        rightPalmMat = glm::rotate(rightPalmMat, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        rightPalmMat = glm::scale(rightPalmMat, glm::vec3(1.20f));

        // Beach utility vehicle and lighthouse landmark.
        g_atvState.pos.y = getCoastalTerrainElevation(g_atvState.pos.x, g_atvState.pos.z);
        glm::mat4 fourWDMat = glm::translate(glm::mat4(1.0f), g_atvState.pos);
        fourWDMat = glm::rotate(fourWDMat, glm::radians(g_atvState.heading), glm::vec3(0.0f, 1.0f, 0.0f));

        // Set the navigation tower on the high north-west shoulder, well above
        // the beach and storm-surge line rather than directly on flat sand.
        const float lighthouseX = -6.0f;
        const float lighthouseZ = -110.0f;
        float lighthouseGround = getCoastalTerrainElevation(lighthouseX, lighthouseZ);
        glm::mat4 lighthouseMat = glm::translate(glm::mat4(1.0f), glm::vec3(lighthouseX, lighthouseGround, lighthouseZ));
        lighthouseMat = glm::rotate(lighthouseMat, glm::radians(8.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Fade the beacon in at dusk, keep it at full power overnight, and
        // fade it out through dawn.
        float lighthouseNightIntensity = 0.0f;
        if (g_dayTime >= 17.75f && g_dayTime < 20.0f)
            lighthouseNightIntensity = glm::smoothstep(17.75f, 20.0f, g_dayTime);
        else if (g_dayTime >= 20.0f || g_dayTime < 4.75f)
            lighthouseNightIntensity = 1.0f;
        else if (g_dayTime < 6.75f)
            lighthouseNightIntensity = 1.0f - glm::smoothstep(4.75f, 6.75f, g_dayTime);

        // ====================================================================
        // RENDER 2: COMPLETE COASTAL ENVIRONMENT SURFACES & BATHYMETRY
        // Expansive seabed sloping down under water, 3D water depth cutout walls,
        // coastal dunes, headlands, backdrop palm forest, and wooden jetties
        // ====================================================================
        // Complete bounded island: submerged shelf, irregular beach ring, and
        // terrain rising gradually toward the central hill.
        drawIslandTerrainBackdrop(shaderProgram, islandTerrainMesh, cubeMesh, sphereMesh);

        // ====================================================================
        // RENDER 3: DYNAMIC PLANAR PROJECTED SHADOWS
        // As the sun traverses its arc, shadows stretch at dawn/dusk,
        // shrink under objects at noon, and sweep across the sand!
        // ====================================================================
        if (g_renderShadows && sunPosition.y > 0.4f)
        {
            glm::mat4 shadowMat = getGroundShadowMatrix(sunPosition, 0.030f);
            glm::vec3 shadowColor(0.20f, 0.16f, 0.12f); // Dark bold ground shadow

            glUniform1i(glGetUniformLocation(shaderProgram.ID, "isShadowPass"), 1);
            glUniform3fv(glGetUniformLocation(shaderProgram.ID, "shadowColor"), 1, glm::value_ptr(shadowColor));

            // Prevent Z-fighting and ensure shadows sit crisp on top of sand
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-2.0f, -2.0f);

            // Umbrella Shadows (Hierarchical canopy discs & tilted poles)
            drawUmbrella(shaderProgram, cubeMesh, sphereMesh, canopyGoreMesh, shadowMat * umbrellaMat1, dynamicUmbrellaTilt, 3.4f, 1.0f);

            // Robot (Person) Shadows
            drawRobot(shaderProgram, cubeMesh, sphereMesh, shadowMat * robotMat1, g_robotHeadYaw, -5.0f, g_robotLeftArm, g_robotRightArm, 82.0f);

            // Beach Lounger Shadows
            drawBeachChair(shaderProgram, cubeMesh, shadowMat * chairMat1);
            for (const BeachChairPlacement& placement : extraChairs)
                drawBeachChair(shaderProgram, cubeMesh, shadowMat * makeChairTransform(placement));

            // Foreground Palm Tree Shadows (Long stretching trunks & feathery fronds)
            drawPalmTree(shaderProgram, cubeMesh, sphereMesh, shadowMat * leftPalmMat, g_palmTrunkCurve, windSway);
            drawPalmTree(shaderProgram, cubeMesh, sphereMesh, shadowMat * backLeftPalm, -g_palmTrunkCurve * 0.75f, windSway);
            drawPalmTree(shaderProgram, cubeMesh, sphereMesh, shadowMat * backRightPalm, g_palmTrunkCurve * 0.85f, windSway);
            drawPalmTree(shaderProgram, cubeMesh, sphereMesh, shadowMat * rightPalmMat, -g_palmTrunkCurve * 0.85f, windSway);

            // Beach 4WD and lighthouse shadows
            drawBeach4WD(shaderProgram, cubeMesh, sphereMesh, wheelCylinderMesh, shadowMat * fourWDMat);
            drawLighthouse(shaderProgram, cubeMesh, sphereMesh, lighthouseTowerMesh,
                           wheelCylinderMesh, lighthouseRoofMesh, shadowMat * lighthouseMat, currentFrame);

            // Realistic Robot Motorboat Shadow on Water Plane
            glm::mat4 waterShadowMat = getGroundShadowMatrix(sunPosition, 0.055f);
            drawRealisticMotorboat(shaderProgram, cubeMesh, sphereMesh, waterShadowMat * boatMat1,
                                   g_boatState.rudderAngle, g_boatState.throttleInput, g_boatState.propellerAngle,
                                   g_boatState.speed, true);

            glDisable(GL_POLYGON_OFFSET_FILL);
            glUniform1i(glGetUniformLocation(shaderProgram.ID, "isShadowPass"), 0);
        }

        // ====================================================================
        // RENDER 4: PRIMARY SCENE OBJECTS (FULL COLOR & LIGHTING)
        // ====================================================================
        // 1. Celestial Sun & Corona are rendered at infinite distance in RENDER 1 (skybox.frag)

        // 2. Coastal Boulders & Rocks (FLAT SHADING for faceted low-detail geometry)
        drawRocks(shaderProgram, cubeMesh, sphereMesh);

        // 3. Foreground Palm Trees (Phong Shading + Swaying Fronds + Palm Leaf Texture)
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, leftPalmMat, g_palmTrunkCurve, windSway, &texManager, g_useTextures);
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, backLeftPalm, -g_palmTrunkCurve * 0.75f, windSway, &texManager, g_useTextures);
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, backRightPalm, g_palmTrunkCurve * 0.85f, windSway, &texManager, g_useTextures);
        drawPalmTree(shaderProgram, cubeMesh, sphereMesh, rightPalmMat, -g_palmTrunkCurve * 0.85f, windSway, &texManager, g_useTextures);

        // Distinct tropical species: broad banana leaves and spiky pandanus
        // crowns break up the repeated coconut-palm silhouette.
        const glm::vec3 bananaPositions[] = {
            glm::vec3(-9.0f, 0.0f, -18.0f), glm::vec3(17.0f, 0.0f, -25.0f),
            glm::vec3(-25.0f, 0.0f, -36.0f), glm::vec3(7.0f, 0.0f, -48.0f),
            glm::vec3(-48.0f, 0.0f, -72.0f), glm::vec3(43.0f, 0.0f, -80.0f),
            glm::vec3(-55.0f, 0.0f, -113.0f), glm::vec3(52.0f, 0.0f, -126.0f),
            glm::vec3(-37.0f, 0.0f, -158.0f), glm::vec3(31.0f, 0.0f, -181.0f)
        };
        for (const glm::vec3& p : bananaPositions)
        {
            glm::mat4 plant = glm::translate(glm::mat4(1.0f),
                glm::vec3(p.x, getCoastalTerrainElevation(p.x, p.z), p.z));
            drawBananaPlant(shaderProgram, cubeMesh, plant, windSway);
        }

        const glm::vec3 pandanusPositions[] = {
            glm::vec3(28.0f, 0.0f, -20.0f), glm::vec3(-20.0f, 0.0f, -47.0f),
            glm::vec3(19.0f, 0.0f, -61.0f), glm::vec3(-31.0f, 0.0f, -70.0f),
            glm::vec3(61.0f, 0.0f, -54.0f), glm::vec3(-62.0f, 0.0f, -91.0f),
            glm::vec3(60.0f, 0.0f, -111.0f), glm::vec3(-53.0f, 0.0f, -139.0f),
            glm::vec3(44.0f, 0.0f, -166.0f), glm::vec3(-24.0f, 0.0f, -196.0f)
        };
        for (const glm::vec3& p : pandanusPositions)
        {
            glm::mat4 tree = glm::translate(glm::mat4(1.0f),
                glm::vec3(p.x, getCoastalTerrainElevation(p.x, p.z), p.z));
            drawPandanusTree(shaderProgram, cubeMesh, tree, windSway);
        }

        // Detached coconuts use staggered deterministic cycles: hang, fall,
        // bounce on the terrain, then reset to a different palm crown.
        drawFallingCoconuts(shaderProgram, sphereMesh, currentFrame);

        // 4. Center Beach Resort Setup (Interactive Robot & Tilting Umbrella)
        drawBeachChair(shaderProgram, cubeMesh, chairMat1);
        for (const BeachChairPlacement& placement : extraChairs)
            drawBeachChair(shaderProgram, cubeMesh, makeChairTransform(placement));
        drawRobot(shaderProgram, cubeMesh, sphereMesh, robotMat1, g_robotHeadYaw, -5.0f, g_robotLeftArm, g_robotRightArm, 82.0f);
        drawUmbrella(shaderProgram, cubeMesh, sphereMesh, canopyGoreMesh, umbrellaMat1, dynamicUmbrellaTilt, 3.4f, 1.0f);

        // 8B. Island transport and navigation landmark
        drawBeach4WD(shaderProgram, cubeMesh, sphereMesh, wheelCylinderMesh, fourWDMat);
        drawLighthouse(shaderProgram, cubeMesh, sphereMesh, lighthouseTowerMesh,
                       wheelCylinderMesh, lighthouseRoofMesh, lighthouseMat, currentFrame);

        // 9. Realistic High-Performance Motorboat Piloted by Articulated Robot + Distant Fleet
        drawRealisticMotorboat(shaderProgram, cubeMesh, sphereMesh, boatMat1,
                               g_boatState.rudderAngle, g_boatState.throttleInput, g_boatState.propellerAngle,
                               g_boatState.speed, true, &texManager, g_useTextures);
        drawBoat(shaderProgram, cubeMesh, sphereMesh, boatMat2, &texManager, g_useTextures);
        drawBoat(shaderProgram, cubeMesh, sphereMesh, boatMat3, &texManager, g_useTextures);

        // The rotating lighthouse beam is rendered after opaque geometry so
        // additive blending produces a visible atmospheric shaft at night.
        drawLighthouseBeacon(shaderProgram, lighthouseBeamMesh, lighthouseMat,
                             currentFrame, lighthouseNightIntensity);

        // 10. Realistic Gerstner Ocean Wave Surface (Semi-Transparent Water with Breaking Crest Foam + Boat Wake)
        drawOceanWaves(waterShader, waterMesh, view, proj, camPos, lightDir, sunLightColor, ambientStrength, currentFrame,
                       &texManager, g_useTextures,
                       g_boatState.pos, g_boatState.heading, g_boatState.speed);

        // ====================================================================
        // DYNAMIC HUD / TITLE BAR UPDATE
        // ====================================================================
        static float lastTitleUpdate = 0.0f;
        if (currentFrame - lastTitleUpdate > 0.08f)
        {
            lastTitleUpdate = currentFrame;
            int hours = (int)g_dayTime;
            int minutes = (int)((g_dayTime - (float)hours) * 60.0f);

            std::string phase = "Day";
            if (g_dayTime >= 5.0f && g_dayTime < 8.0f)        phase = "Sunrise";
            else if (g_dayTime >= 8.0f && g_dayTime < 16.5f) phase = "Midday";
            else if (g_dayTime >= 16.5f && g_dayTime < 19.5f) phase = "Sunset";
            else phase = "Night";

            std::string shadeStr = (g_shadingMode == 0) ? "Auto Shading" :
                                   (g_shadingMode == 1) ? "Flat" :
                                   (g_shadingMode == 2) ? "Gouraud" : "Phong";

            std::ostringstream ss;
            if (g_boatPilotMode)
            {
                float speedKnots = g_boatState.speed * 1.94384f;
                int headingDeg = (int)(fmodf(g_boatState.heading + 3600.0f, 360.0f));
                ss << ">>> [ROBOT BOAT PILOT MODE] <<<  Speed: "
                   << std::fixed << std::setprecision(1) << speedKnots << " kt (" << g_boatState.speed << " m/s) | Heading: "
                   << std::setw(3) << std::setfill('0') << headingDeg << " deg | Rudder: "
                   << std::showpos << (int)g_boatState.rudderAngle << " deg | Controls: WASD/Arrows | [F] Exit Pilot";
            }
            else if (g_atvDriveMode)
            {
                ss << ">>> [BEACH ATV DRIVE MODE] <<<  Speed: "
                   << std::fixed << std::setprecision(1) << fabsf(g_atvState.speed) * 3.6f
                   << " km/h | Controls: WASD | [H] Exit ATV";
            }
            else
            {
                ss << "Beach Day 3D | "
                   << std::setfill('0') << std::setw(2) << hours << ":"
                   << std::setfill('0') << std::setw(2) << minutes << " (" << phase << ") "
                   << (g_dayPaused ? "[PAUSED]" : "[Speed: " + std::to_string((int)(g_daySpeed * 10.0f) / 10.0f).substr(0, 3) + "x]")
                   << " | " << shadeStr
                   << " | [H] Drive ATV | [F] Pilot Robot Boat | [B] Focus Boat | [1-4] Views";
            }

            glfwSetWindowTitle(window, ss.str().c_str());
        }

        // Swap Buffers and Poll Events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8. Resource Cleanup
    cubeMesh.cleanup();
    sphereMesh.cleanup();
    canopyGoreMesh.cleanup();
    waterMesh.cleanup();
    seabedMesh.cleanup();
    waterSkirtMesh.cleanup();
    islandTerrainMesh.cleanup();
    wheelCylinderMesh.cleanup();
    lighthouseTowerMesh.cleanup();
    lighthouseRoofMesh.cleanup();
    lighthouseBeamMesh.cleanup();
    skybox.cleanup();
    texManager.cleanup();
    shaderProgram.Delete();
    waterShader.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// ============================================================================
// INPUT PROCESSING (INTERACTIVE DAY-CYCLE, SHADING, TEXTURES & CAMERA)
// ============================================================================
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
        zKeyPressed = false;

    // Toggle Day-Cycle Pause
    static bool spacePressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spacePressed)
    {
        g_dayPaused = !g_dayPaused;
        std::cout << ">>> [Day Cycle] " << (g_dayPaused ? "PAUSED" : "RUNNING") << " <<<\n";
        spacePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        spacePressed = false;

    // Day-Cycle Speed Control: [ decreases speed, ] increases speed
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        g_daySpeed = glm::max(0.05f, g_daySpeed - 0.25f * deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        g_daySpeed = glm::min(4.0f, g_daySpeed + 0.25f * deltaTime);
    }

    // Direct Time Jump Keys:
    // J or F5: Sunrise (06:30)
    static bool jPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) && !jPressed)
    {
        g_dayTime = 6.5f;
        std::cout << ">>> [Time Jump] Sunrise (06:30) - Warm Golden Amber Light <<<\n";
        jPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE)
        jPressed = false;

    // K or F6: Midday (12:00)
    static bool kPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS) && !kPressed)
    {
        g_dayTime = 12.0f;
        std::cout << ">>> [Time Jump] Midday (12:00) - Brilliant White Daylight <<<\n";
        kPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_F6) == GLFW_RELEASE)
        kPressed = false;

    // L or F7: Sunset (18:15)
    static bool lKeyTimePressed = false;
    if ((glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS) && !lKeyTimePressed)
    {
        g_dayTime = 18.25f;
        std::cout << ">>> [Time Jump] Sunset (18:15) - Fiery Crimson/Orange Dusk <<<\n";
        lKeyTimePressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F7) == GLFW_RELEASE)
        lKeyTimePressed = false;

    // N or F8: Twilight / Night (21:30)
    static bool nPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_F8) == GLFW_PRESS) && !nPressed)
    {
        g_dayTime = 21.5f;
        std::cout << ">>> [Time Jump] Night (21:30) - Midnight Starry Sky & Moonlight <<<\n";
        nPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_F8) == GLFW_RELEASE)
        nPressed = false;

    // Cycle Projection Mode: V key (Perspective -> Ortho Top View -> Ortho Side View)
    static bool vPressed = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !vPressed)
    {
        camera.cycleProjectionMode();
        vPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
        vPressed = false;

    // Cycle Shading Mode Override: M key
    static bool mPressed = false;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mPressed)
    {
        g_shadingMode = (g_shadingMode + 1) % 4;
        const char* modeNames[] = {
            "Auto (Proposal: Rocks=Flat, Sea/Beach=Gouraud, Objects=Phong)",
            "Force ALL Flat Shading (Faceted Low-Poly)",
            "Force ALL Gouraud Shading (Smooth Per-Vertex)",
            "Force ALL Phong Shading (Per-Fragment Highlights)"
        };
        std::cout << "\n>>> [Shading Mode] " << modeNames[g_shadingMode] << " <<<\n" << std::endl;
        mPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
        mPressed = false;

    // Toggle Texture Mapping: X key
    static bool xPressed = false;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && !xPressed)
    {
        g_useTextures = !g_useTextures;
        std::cout << ">>> [Texture Mapping] " << (g_useTextures ? "ENABLED" : "DISABLED") << " <<<\n";
        xPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE)
        xPressed = false;

    // Toggle Dynamic Shadows: G key
    static bool gPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed)
    {
        g_renderShadows = !g_renderShadows;
        std::cout << ">>> [Dynamic Projected Shadows] " << (g_renderShadows ? "ENABLED" : "DISABLED") << " <<<\n";
        gPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
        gPressed = false;

    // Interactive Parameters: Keys 1/2 for Robot Head, Keys 3/4 for Umbrella Tilt
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        g_robotHeadYaw -= 45.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        g_robotHeadYaw += 45.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        g_umbrellaTilt = glm::max(0.0f, g_umbrellaTilt - 20.0f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        g_umbrellaTilt = glm::min(40.0f, g_umbrellaTilt + 20.0f * deltaTime);

    // Toggle Robot Boat Pilot Mode: Key F or Key 0
    static bool fKeyPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) && !fKeyPressed)
    {
        g_boatPilotMode = !g_boatPilotMode;
        if (g_boatPilotMode)
        {
            g_atvDriveMode = false;
            camera.isTransitioning = false;
            std::cout << "\n=======================================================\n";
            std::cout << ">>> [ROBOT BOAT PILOT MODE: ACTIVATED] <<<\n";
            std::cout << "    [W] / [UP]     : Throttle Forward (Accelerate)\n";
            std::cout << "    [S] / [DOWN]   : Throttle Reverse / Brake\n";
            std::cout << "    [A] / [LEFT]   : Steer Port (Left) - Robot steers & glances left\n";
            std::cout << "    [D] / [RIGHT]  : Steer Starboard (Right) - Robot steers & glances right\n";
            std::cout << "    [F] / [0]      : Exit Pilot Mode (Return to Free Camera)\n";
            std::cout << "=======================================================\n\n";
        }
        else
        {
            std::cout << "\n>>> [ROBOT BOAT PILOT MODE: DEACTIVATED] - Free Camera Mode <<<\n\n";
        }
        fKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE)
        fKeyPressed = false;

    // Toggle the beach-ring ATV chase camera and driving controls.
    static bool hKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed)
    {
        g_atvDriveMode = !g_atvDriveMode;
        if (g_atvDriveMode)
        {
            g_boatPilotMode = false;
            camera.isTransitioning = false;
            std::cout << "\n>>> [BEACH ATV DRIVE MODE: ACTIVATED] <<<\n";
            std::cout << "    [W/S] : Accelerate / Reverse\n";
            std::cout << "    [A/D] : Steer around the beach ring\n";
            std::cout << "    [H]   : Exit ATV mode\n";
        }
        else
        {
            g_atvState.speed = 0.0f;
            std::cout << "\n>>> [BEACH ATV DRIVE MODE: DEACTIVATED] <<<\n";
        }
        hKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
        hKeyPressed = false;

    // Quick Camera Presets
    static bool bKeyPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) && !bKeyPressed)
    {
        camera.setPreset(VIEW_BOAT, window);
        camera.desiredTarget = g_boatState.pos + glm::vec3(0.0f, 1.1f, 0.0f);
        camera.desiredDistance = 8.5f;
        bKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
        bKeyPressed = false;

    static bool tKeyPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) && !tKeyPressed)
    {
        camera.setPreset(VIEW_TOWER, window);
        tKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE)
        tKeyPressed = false;

    static bool cKeyPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) && !cKeyPressed)
    {
        camera.setPreset(VIEW_ROBOT, window);
        cKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE)
        cKeyPressed = false;

    static bool pKeyPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) && !pKeyPressed)
    {
        camera.setPreset(VIEW_PALM_ROCKS, window);
        pKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE)
        pKeyPressed = false;

    static bool lCamPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) && !lCamPressed)
    {
        camera.setPreset(VIEW_RIGHT_BEACH, window);
        lCamPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE)
        lCamPressed = false;

    static bool oKeyPressed = false;
    if ((glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) && !oKeyPressed)
    {
        camera.setPreset(VIEW_OVERVIEW, window);
        oKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE)
        oKeyPressed = false;

    static bool tabKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabKeyPressed)
    {
        camera.cycleNextPreset(window);
        tabKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
        tabKeyPressed = false;
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
