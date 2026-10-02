// ============================================================================
// CSE 4102 Computer Graphics - Raised Tropical Stilt House
// Stage 1: Window, Shaders, Camera, Cube Mesh, Ground
// Modern OpenGL 3.3 Core Profile (C++, GLFW, GLAD, GLM)
// ============================================================================

#include <iostream>
#include <vector>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ============================================================================
// NAMED CONSTANTS (House Dimensions in meters, Y is up, 1 unit = 1 meter)
// ============================================================================
const float PILLAR_H       = 1.80f;  // Height of foundation stilts
const float PILLAR_W       = 0.20f;  // Thickness of wooden stilts
const float FLOOR_W        = 8.00f;  // Width of raised deck (X axis)
const float FLOOR_D        = 5.00f;  // Depth of raised deck (Z axis)
const float FLOOR_T        = 0.25f;  // Deck thickness
const float WALL_H         = 2.20f;  // Wall height
const float ROOF_OVERHANG  = 1.20f;  // Eaves overhang beyond walls
const float ROOF_PITCH_DEG = 40.0f;  // Roof slope angle in degrees
const int   STAIR_STEPS    = 12;     // Steps for tall left staircase
const float STEP_H         = 0.17f;  // Step riser height
const float STEP_D         = 0.30f;  // Step tread depth

// Window dimensions
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

// ============================================================================
// MESH STRUCT
// ============================================================================
struct Mesh
{
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLsizei vertexCount = 0;

    void cleanup()
    {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        VAO = 0;
        VBO = 0;
        vertexCount = 0;
    }
};

// Global shader uniform locations
struct ShaderLocations
{
    GLuint program = 0;
    GLint uModel = -1;
    GLint uView = -1;
    GLint uProjection = -1;
    GLint uNormalMatrix = -1;
    GLint uColor = -1;
    GLint uLightDir = -1;
    GLint uViewPos = -1;
} g_shader;

// ============================================================================
// CAMERA & INPUT STATE
// ============================================================================
glm::vec3 cameraPos   = glm::vec3(0.0f, 3.5f, 15.0f);
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 1.5f, 0.0f) - cameraPos);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

float cameraYaw   = -90.0f;
float cameraPitch = -8.0f;
float cameraFov   = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool isRightMousePressed = false;

// ============================================================================
// SHADER SOURCE CODE (Embedded GLSL 330 core)
// ============================================================================
const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = normalize(uNormalMatrix * aNormal);
    gl_Position = uProjection * uView * worldPos;
}
)";

const char* fragmentShaderSource = R"(#version 330 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 uColor;
uniform vec3 uLightDir; // Normalized direction toward the sun
uniform vec3 uViewPos;

out vec4 FragColor;

void main()
{
    // ========================================================================
    // PHONG LIGHTING MODEL:
    // Color = (Ambient + Diffuse + Specular) * SurfaceColor
    //
    // 1. Ambient  = ambientStrength * lightColor
    // 2. Diffuse  = max(dot(Normal, LightDir), 0.0) * lightColor
    // 3. Specular = specularStrength * pow(max(dot(ViewDir, ReflectDir), 0.0), shininess) * lightColor
    // ========================================================================

    vec3 lightColor = vec3(1.0, 0.98, 0.92); // Warm tropical sunlight
    vec3 N = normalize(Normal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uViewPos - FragPos);
    vec3 R = reflect(-L, N);

    // Ambient Component
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse Component (Lambertian)
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular Component (Phong)
    float specularStrength = 0.20;
    float shininess = 16.0;
    float spec = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Final blended fragment color
    vec3 result = (ambient + diffuse + specular) * uColor;
    FragColor = vec4(result, 1.0);
}
)";

// Compile and link shaders
GLuint createShaderProgram(const char* vSource, const char* fSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// ============================================================================
// PRIMITIVE MESH GENERATORS
// ============================================================================

// Unit cube centered at origin [-0.5, 0.5]^3 (36 vertices with face normals, CCW)
Mesh createCube()
{
    Mesh mesh;
    // 36 vertices: 6 faces * 2 triangles * 3 vertices = 36 vertices
    // Format: posX, posY, posZ, normX, normY, normZ
    const float vertices[] = {
        // Front Face (Normal: 0, 0, 1)
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        // Back Face (Normal: 0, 0, -1)
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

        // Left Face (Normal: -1, 0, 0)
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,

        // Right Face (Normal: 1, 0, 0)
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

        // Top Face (Normal: 0, 1, 0)
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,

        // Bottom Face (Normal: 0, -1, 0)
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f
    };

    mesh.vertexCount = 36;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return mesh;
}

// Global mesh primitives
Mesh g_cubeMesh;

// ============================================================================
// DRAW HELPER FUNCTION
// ============================================================================
void drawPrimitive(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color)
{
    glUniformMatrix4fv(g_shader.uModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(g_shader.uColor, 1, glm::value_ptr(color));

    // Normal matrix transforms surface normals correctly under non-uniform scaling
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    glUniformMatrix3fv(g_shader.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    glBindVertexArray(mesh.VAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    glBindVertexArray(0);
}

// ============================================================================
// STAGE 1: GROUND PLANE
// ============================================================================
void drawGround()
{
    // Flat cube 30m x 0.1m x 30m, top surface placed at y = 0.0m
    // Center at y = -0.05m so top face touches y = 0.0
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.05f, 0.0f));
    model = glm::scale(model, glm::vec3(30.0f, 0.10f, 30.0f));
    
    // Light gray sandy gravel ground color
    drawPrimitive(g_cubeMesh, model, glm::vec3(0.70f, 0.70f, 0.70f));
}

// ============================================================================
// CALLBACKS & INPUT HANDLING
// ============================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            isRightMousePressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (action == GLFW_RELEASE)
        {
            isRightMousePressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (!isRightMousePressed)
    {
        lastX = xpos;
        lastY = ypos;
        return;
    }

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.12f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    cameraYaw   += xoffset;
    cameraPitch += yoffset;

    // Constrain pitch to avoid screen flip
    if (cameraPitch > 89.0f)  cameraPitch = 89.0f;
    if (cameraPitch < -89.0f) cameraPitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraFov -= (float)yoffset * 2.0f;
    if (cameraFov < 15.0f) cameraFov = 15.0f;
    if (cameraFov > 80.0f) cameraFov = 80.0f;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 6.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 2.0f;

    glm::vec3 flatFront = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 right = glm::normalize(glm::cross(flatFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += flatFront * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= flatFront * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= right * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += right * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraUp * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= cameraUp * cameraSpeed;

    // Minimum camera height above ground
    if (cameraPos.y < 0.3f) cameraPos.y = 0.3f;
}

// ============================================================================
// MAIN APPLICATION
// ============================================================================
int main()
{
    std::cout << "=======================================================\n";
    std::cout << "  3D RAISED TROPICAL STILT HOUSE - STAGE 1\n";
    std::cout << "  Window, Phong Shaders, Orbit/Fly Camera, Cube, Ground\n";
    std::cout << "=======================================================\n";
    std::cout << "  Controls:\n";
    std::cout << "    [W, A, S, D]        : Fly / Walk around\n";
    std::cout << "    [E / Space]         : Move Up\n";
    std::cout << "    [Q / Ctrl]          : Move Down\n";
    std::cout << "    [Right Mouse Drag]  : Look around / Orbit\n";
    std::cout << "    [Mouse Scroll]      : Zoom in / out (adjust FOV)\n";
    std::cout << "    [Shift]             : Boost speed\n";
    std::cout << "    [Esc]               : Exit\n";
    std::cout << "=======================================================\n";

    // 1. Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Raised Tropical Stilt House - Stage 1", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 2. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    // 3. Configure OpenGL State
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Tropical sky clear color: RGB(0.6, 0.8, 1.0)
    glClearColor(0.60f, 0.80f, 1.0f, 1.0f);

    // 4. Compile Shaders & Cache Uniform Locations
    g_shader.program       = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    g_shader.uModel        = glGetUniformLocation(g_shader.program, "uModel");
    g_shader.uView         = glGetUniformLocation(g_shader.program, "uView");
    g_shader.uProjection   = glGetUniformLocation(g_shader.program, "uProjection");
    g_shader.uNormalMatrix = glGetUniformLocation(g_shader.program, "uNormalMatrix");
    g_shader.uColor        = glGetUniformLocation(g_shader.program, "uColor");
    g_shader.uLightDir     = glGetUniformLocation(g_shader.program, "uLightDir");
    g_shader.uViewPos      = glGetUniformLocation(g_shader.program, "uViewPos");

    // 5. Create Cube Mesh
    g_cubeMesh = createCube();

    // Sunlight Direction (-0.4, -1.0, -0.3) normalized pointing towards sun: (0.4, 1.0, 0.3)
    glm::vec3 sunLightDir = glm::normalize(glm::vec3(0.40f, 1.0f, 0.30f));

    // Main Render Loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Clear Color and Depth Buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(g_shader.program);

        // View and Projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        float aspect = (fbHeight > 0) ? (float)fbWidth / (float)fbHeight : (float)SCR_WIDTH / (float)SCR_HEIGHT;
        glm::mat4 proj = glm::perspective(glm::radians(cameraFov), aspect, 0.1f, 100.0f);

        glUniformMatrix4fv(g_shader.uView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(g_shader.uProjection, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(g_shader.uLightDir, 1, glm::value_ptr(sunLightDir));
        glUniform3fv(g_shader.uViewPos, 1, glm::value_ptr(cameraPos));

        // Render Ground Plane
        drawGround();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup resources
    g_cubeMesh.cleanup();
    glDeleteProgram(g_shader.program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
