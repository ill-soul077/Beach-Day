#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

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

    bool firstMouse;
    double lastX;
    double lastY;
    bool isDraggingOrbit;
    bool isDraggingPan;

    Camera(glm::vec3 targetPos = glm::vec3(0.0f, 1.0f, 0.0f), float dist = 12.0f)
        : target(targetPos), distance(dist), yaw(45.0f), pitch(22.0f),
          fov(45.0f), nearPlane(0.1f), farPlane(300.0f),
          firstMouse(true), lastX(0.0), lastY(0.0),
          isDraggingOrbit(false), isDraggingPan(false)
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
        return glm::lookAt(getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) const
    {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }

    void handleScroll(double yoffset)
    {
        distance -= (float)yoffset * 0.8f;
        if (distance < 1.0f) distance = 1.0f;
        if (distance > 120.0f) distance = 120.0f;
    }

    void handleMouseButton(int button, int action, double xpos, double ypos)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                isDraggingOrbit = true;
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
            float xoffset = (float)(xpos - lastX);
            float yoffset = (float)(ypos - lastY);
            lastX = xpos;
            lastY = ypos;

            float panSpeed = 0.008f * distance;
            // Pan horizontally and vertically relative to view
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
            distance = glm::max(1.0f, distance - speed);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            distance = glm::min(120.0f, distance + speed);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            yaw -= 45.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            yaw += 45.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            // Reset camera
            target = glm::vec3(0.0f, 1.0f, 0.0f);
            distance = 12.0f;
            yaw = 45.0f;
            pitch = 22.0f;
        }
    }
};

#endif // CAMERA_H
