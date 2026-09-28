goit#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaderClass.h"

// ============================================================================
// DYNAMIC PROCEDURAL SKYBOX (SUNRISE TO SUNSET HORIZON & BACKGROUND)
// Fulfills Proposal Requirement:
// "Skybox implementation for a realistic, seamless horizon and background"
// Renders an infinite sky dome with real-time zenith/horizon color shifts,
// atmospheric solar glare, and twinkling night stars.
// ============================================================================
class Skybox
{
public:
    GLuint VAO, VBO;
    Shader shader;

    Skybox() : VAO(0), VBO(0), shader("skybox.vert", "skybox.frag")
    {
        // 36 vertices of a cube surrounding the world
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void draw(const glm::mat4& view, const glm::mat4& proj, float timeOfDay, const glm::vec3& sunDir)
    {
        // Calculate sky palette based on time of day
        glm::vec3 zenithColor;
        glm::vec3 horizonColor;
        glm::vec3 oceanHorizonColor;
        glm::vec3 sunGlowColor;

        if (timeOfDay >= 5.0f && timeOfDay < 7.5f)
        {
            // Sunrise (Dawn transition: 5:00 - 7:30)
            float t = (timeOfDay - 5.0f) / 2.5f;
            zenithColor = glm::mix(glm::vec3(0.04f, 0.08f, 0.22f), glm::vec3(0.20f, 0.42f, 0.75f), t);
            horizonColor = glm::mix(glm::vec3(0.70f, 0.28f, 0.18f), glm::vec3(0.98f, 0.65f, 0.38f), t);
            oceanHorizonColor = glm::mix(glm::vec3(0.06f, 0.18f, 0.28f), glm::vec3(0.12f, 0.44f, 0.58f), t);
            sunGlowColor = glm::vec3(1.0f, 0.60f, 0.25f);
        }
        else if (timeOfDay >= 7.5f && timeOfDay < 16.5f)
        {
            // Midday (Clear bright tropical coastal sky: 7:30 - 16:30)
            zenithColor = glm::vec3(0.18f, 0.48f, 0.88f);       // Deep brilliant azure
            horizonColor = glm::vec3(0.68f, 0.84f, 0.96f);      // Soft coastal haze
            oceanHorizonColor = glm::vec3(0.10f, 0.48f, 0.65f); // Tropical teal blue
            sunGlowColor = glm::vec3(1.0f, 0.95f, 0.80f);
        }
        else if (timeOfDay >= 16.5f && timeOfDay < 19.5f)
        {
            // Sunset (Dusk transition: 16:30 - 19:30)
            float t = (timeOfDay - 16.5f) / 3.0f;
            zenithColor = glm::mix(glm::vec3(0.18f, 0.48f, 0.88f), glm::vec3(0.14f, 0.08f, 0.28f), t);
            horizonColor = glm::mix(glm::vec3(0.95f, 0.60f, 0.30f), glm::vec3(0.95f, 0.25f, 0.12f), t);
            oceanHorizonColor = glm::mix(glm::vec3(0.10f, 0.48f, 0.65f), glm::vec3(0.15f, 0.10f, 0.25f), t);
            sunGlowColor = glm::vec3(1.0f, 0.45f, 0.18f);
        }
        else
        {
            // Night (Twilight / Midnight: 19:30 - 5:00)
            zenithColor = glm::vec3(0.015f, 0.025f, 0.08f);     // Midnight navy
            horizonColor = glm::vec3(0.04f, 0.06f, 0.15f);      // Deep horizon blue
            oceanHorizonColor = glm::vec3(0.02f, 0.04f, 0.10f); // Dark sea
            sunGlowColor = glm::vec3(0.25f, 0.35f, 0.55f);      // Soft moonlight
        }

        // Change depth function so depth test passes when values are equal to depth buffer's content
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE); // Don't write to depth buffer so scene geometry renders on top

        shader.Activate();
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.ID, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(glGetUniformLocation(shader.ID, "zenithColor"), 1, glm::value_ptr(zenithColor));
        glUniform3fv(glGetUniformLocation(shader.ID, "horizonColor"), 1, glm::value_ptr(horizonColor));
        glUniform3fv(glGetUniformLocation(shader.ID, "oceanHorizonColor"), 1, glm::value_ptr(oceanHorizonColor));
        glUniform3fv(glGetUniformLocation(shader.ID, "sunDir"), 1, glm::value_ptr(sunDir));
        glUniform3fv(glGetUniformLocation(shader.ID, "sunGlowColor"), 1, glm::value_ptr(sunGlowColor));
        glUniform1f(glGetUniformLocation(shader.ID, "timeOfDay"), timeOfDay);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); // Restore default depth func
    }

    void cleanup()
    {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        shader.Delete();
    }
};

#endif // SKYBOX_H
