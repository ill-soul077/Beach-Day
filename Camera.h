#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>
#include <string>

enum ViewPreset
{
    VIEW_OVERVIEW = 0,
    VIEW_BOAT,
    VIEW_TOWER,
    VIEW_ROBOT,
    VIEW_PALM_ROCKS,
    VIEW_RIGHT_BEACH,
    VIEW_SEA_BOATS,
    VIEW_PRESET_COUNT
};

enum ProjectionMode
{
    PROJ_PERSPECTIVE = 0,
    PROJ_ORTHO_TOP,
    PROJ_ORTHO_SIDE,
    PROJ_MODE_COUNT
};

class Camera
{
public:
    glm::vec3 target;
    float distance;
    float yaw;          // in degrees
    float pitch;        // in degrees

    float fov;
    float nearPlane;
    float farPlane;

    ProjectionMode projMode;
    float orthoSize;

    bool firstMouse;
    double lastX;
    double lastY;
    bool isDraggingOrbit;
    bool isDraggingPan;

    // Smooth transition / presets
    glm::vec3 desiredTarget;
    float desiredDistance;
    float desiredYaw;
    float desiredPitch;
    bool isTransitioning;
    int currentPresetIndex;

    Camera(glm::vec3 targetPos = glm::vec3(0.0f, 1.0f, 0.0f), float dist = 12.0f)
        : target(targetPos), distance(dist), yaw(45.0f), pitch(22.0f),
          fov(45.0f), nearPlane(0.1f), farPlane(2000.0f),
          projMode(PROJ_PERSPECTIVE), orthoSize(22.0f),
          firstMouse(true), lastX(0.0), lastY(0.0),
          isDraggingOrbit(false), isDraggingPan(false),
          desiredTarget(targetPos), desiredDistance(dist), desiredYaw(45.0f), desiredPitch(22.0f),
          isTransitioning(false), currentPresetIndex(0)
    {
    }

    glm::vec3 getPosition() const
    {
        float radPitch = glm::radians(pitch);
        float radYaw = glm::radians(yaw);

        float x = target.x + distance * cosf(radPitch) * sinf(radYaw);
        float y = target.y + distance * sinf(radPitch);
        float z = target.z + distance * cosf(radPitch) * cosf(radYaw);

        return glm::vec3(x, y, z);
    }

    glm::mat4 getViewMatrix() const
    {
        if (projMode == PROJ_ORTHO_TOP)
        {
            // Orthographic Top View looking straight down at the beach layout
            return glm::lookAt(glm::vec3(target.x, target.y + 45.0f, target.z + 0.001f),
                               target,
                               glm::vec3(0.0f, 0.0f, -1.0f));
        }
        else if (projMode == PROJ_ORTHO_SIDE)
        {
            // Orthographic Side View looking along coastal profile
            return glm::lookAt(glm::vec3(target.x + 45.0f, target.y + 2.0f, target.z),
                               target,
                               glm::vec3(0.0f, 1.0f, 0.0f));
        }
        return glm::lookAt(getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) const
    {
        if (projMode == PROJ_PERSPECTIVE)
        {
            return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }
        else
        {
            float halfWidth = orthoSize * aspectRatio;
            float halfHeight = orthoSize;
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -500.0f, 2000.0f);
        }
    }

    void cycleProjectionMode()
    {
        projMode = (ProjectionMode)((projMode + 1) % PROJ_MODE_COUNT);
        std::cout << "\n>>> [Camera Projection Mode] "
                  << (projMode == PROJ_PERSPECTIVE ? "Perspective Projection (Eye Depth View)" :
                      projMode == PROJ_ORTHO_TOP ? "Orthographic Projection (Top Plan View)" :
                      "Orthographic Projection (Side Elevation View)")
                  << " <<<\n" << std::endl;
    }

    void update(float deltaTime)
    {
        if (isTransitioning)
        {
            float t = glm::clamp(deltaTime * 5.0f, 0.0f, 1.0f);
            target = glm::mix(target, desiredTarget, t);
            distance = glm::mix(distance, desiredDistance, t);

            // Angle difference with 360-degree wrap-around handling
            float yawDiff = desiredYaw - yaw;
            while (yawDiff < -180.0f) yawDiff += 360.0f;
            while (yawDiff > 180.0f) yawDiff -= 360.0f;
            yaw += yawDiff * t;

            pitch = glm::mix(pitch, desiredPitch, t);

            if (glm::distance(target, desiredTarget) < 0.02f &&
                std::fabs(distance - desiredDistance) < 0.02f &&
                std::fabs(yawDiff) < 0.2f &&
                std::fabs(pitch - desiredPitch) < 0.2f)
            {
                target = desiredTarget;
                distance = desiredDistance;
                yaw = desiredYaw;
                pitch = desiredPitch;
                isTransitioning = false;
            }
        }
    }

    void setPreset(ViewPreset preset, GLFWwindow* window = nullptr)
    {
        currentPresetIndex = (int)preset;
        std::string name = "";

        switch (preset)
        {
        case VIEW_BOAT:
            // Focus directly on the sailboat floating in the water
            desiredTarget = glm::vec3(4.2f, 1.6f, 14.0f);
            desiredDistance = 6.5f;
            desiredYaw = 20.0f;
            desiredPitch = 14.0f;
            name = "Sailboat [Key: B]";
            break;

        case VIEW_TOWER:
            // Focus on the Lifeguard Watch Tower on the shoreline
            desiredTarget = glm::vec3(-4.2f, 2.2f, 3.2f);
            desiredDistance = 8.0f;
            desiredYaw = 22.0f;
            desiredPitch = 16.0f;
            name = "Lifeguard Watch Tower [Key: T]";
            break;

        case VIEW_ROBOT:
            // Focus on the Center Robot lounging under umbrella
            desiredTarget = glm::vec3(0.0f, 1.0f, -0.8f);
            desiredDistance = 5.2f;
            desiredYaw = 38.0f;
            desiredPitch = 18.0f;
            name = "Center Robot & Umbrella [Key: C]";
            break;

        case VIEW_PALM_ROCKS:
            // Focus on the Left Palm Trees and Coastal Rocks
            desiredTarget = glm::vec3(-13.5f, 2.8f, -4.0f);
            desiredDistance = 9.5f;
            desiredYaw = 42.0f;
            desiredPitch = 16.0f;
            name = "Palm Trees & Rocks [Key: P]";
            break;

        case VIEW_RIGHT_BEACH:
            // Focus on the Right Beach Lounge Setup
            desiredTarget = glm::vec3(11.0f, 1.2f, -1.8f);
            desiredDistance = 8.5f;
            desiredYaw = -32.0f;
            desiredPitch = 16.0f;
            name = "East Shore Loungers [Key: L / 9]";
            break;

        case VIEW_SEA_BOATS:
            // Focus on the Open Sea and Sailing Fleet
            desiredTarget = glm::vec3(0.0f, 1.8f, 20.0f);
            desiredDistance = 18.0f;
            desiredYaw = 10.0f;
            desiredPitch = 12.0f;
            name = "Ocean Sailing Fleet [Key: 7]";
            break;

        case VIEW_OVERVIEW:
        default:
            // Ocean-facing establishing shot: sea -> beach -> coconut belt ->
            // low forested island ridge, matching the Saint Martin reference.
            desiredTarget = glm::vec3(0.0f, 12.0f, -92.0f);
            desiredDistance = 355.0f;
            desiredYaw = 0.0f;
            desiredPitch = 42.0f;
            name = "Saint Martin Island Panorama [Key: O / R]";
            break;
        }

        isTransitioning = true;
        std::cout << "\n>>> [Camera View Preset] " << name << " <<<\n"
                  << "    Target: (" << desiredTarget.x << ", " << desiredTarget.y << ", " << desiredTarget.z << ")"
                  << " | Distance: " << desiredDistance << "m\n" << std::endl;

        if (window)
        {
            std::string title = "Beach Day 3D - Focused on: " + name;
            glfwSetWindowTitle(window, title.c_str());
        }
    }

    void cycleNextPreset(GLFWwindow* window = nullptr)
    {
        int next = (currentPresetIndex + 1) % VIEW_PRESET_COUNT;
        setPreset((ViewPreset)next, window);
    }

    void handleScroll(double yoffset)
    {
        isTransitioning = false;
        distance -= (float)yoffset * 0.8f;
        if (distance < 1.0f) distance = 1.0f;
        if (distance > 600.0f) distance = 600.0f;
    }

    void handleMouseButton(int button, int action, double xpos, double ypos)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                isDraggingOrbit = true;
                isTransitioning = false;
                lastX = xpos;
                lastY = ypos;
            }
            else if (action == GLFW_RELEASE)
            {
                isDraggingOrbit = false;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                isDraggingPan = true;
                isTransitioning = false;
                lastX = xpos;
                lastY = ypos;
            }
            else if (action == GLFW_RELEASE)
            {
                isDraggingPan = false;
            }
        }
    }

    void handleMouseMove(double xpos, double ypos)
    {
        if (isDraggingOrbit)
        {
            isTransitioning = false;
            float xoffset = (float)(xpos - lastX);
            float yoffset = (float)(lastY - ypos);
            lastX = xpos;
            lastY = ypos;

            float sensitivity = 0.3f;
            yaw -= xoffset * sensitivity;
            pitch += yoffset * sensitivity;

            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -10.0f) pitch = -10.0f;
        }
        else if (isDraggingPan)
        {
            isTransitioning = false;
            float xoffset = (float)(xpos - lastX);
            float yoffset = (float)(ypos - lastY);
            lastX = xpos;
            lastY = ypos;

            float panSpeed = 0.008f * distance;
            glm::vec3 camPos = getPosition();
            glm::vec3 forward = glm::normalize(target - camPos);
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up = glm::cross(right, forward);

            target -= right * (xoffset * panSpeed);
            target += up * (yoffset * panSpeed);
        }
    }

    void handleKeyboard(GLFWwindow* window, float deltaTime)
    {
        float speed = 8.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            isTransitioning = false;
            distance = glm::max(1.0f, distance - speed);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            isTransitioning = false;
            distance = glm::min(600.0f, distance + speed);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            isTransitioning = false;
            yaw -= 45.0f * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            isTransitioning = false;
            yaw += 45.0f * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            setPreset(VIEW_OVERVIEW, window);
        }

        // Arrow Key Pan Navigation across the 3D beach plane
        float panSpeed = 10.0f * deltaTime;
        float radYaw = glm::radians(yaw);
        glm::vec3 forward(-sinf(radYaw), 0.0f, -cosf(radYaw));
        glm::vec3 right(cosf(radYaw), 0.0f, -sinf(radYaw));

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            isTransitioning = false;
            target += forward * panSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            isTransitioning = false;
            target -= forward * panSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            isTransitioning = false;
            target -= right * panSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            isTransitioning = false;
            target += right * panSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        {
            isTransitioning = false;
            target.y += panSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        {
            isTransitioning = false;
            target.y = glm::max(0.0f, target.y - panSpeed);
        }
    }
};

#endif // CAMERA_H
