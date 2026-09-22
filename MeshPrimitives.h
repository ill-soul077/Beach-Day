#ifndef MESH_PRIMITIVES_H
#define MESH_PRIMITIVES_H

#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>

// ============================================================================
// BASE PRIMITIVE: CUBE MESH
// Created by stitching 6 square faces (24 vertices, 36 indices)
// centered at origin [-0.5, 0.5]^3 with exact outward normals for each face.
// ============================================================================
class CubeMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    CubeMesh() : VAO(0), VBO(0), EBO(0), indexCount(36)
    {
        // 24 vertices: (pos.x, pos.y, pos.z, norm.x, norm.y, norm.z, tex.u, tex.v)
        GLfloat vertices[] = {
            // Front Face (Normal: 0, 0, 1)
            -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,

            // Back Face (Normal: 0, 0, -1)
             0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,

            // Left Face (Normal: -1, 0, 0)
            -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

            // Right Face (Normal: 1, 0, 0)
             0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

            // Top Face (Normal: 0, 1, 0)
            -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,

            // Bottom Face (Normal: 0, -1, 0)
            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f
        };

        GLuint indices[] = {
            0,  1,  2,      2,  3,  0,      // Front
            4,  5,  6,      6,  7,  4,      // Back
            8,  9,  10,     10, 11, 8,      // Left
            12, 13, 14,     14, 15, 12,     // Right
            16, 17, 18,     18, 19, 16,     // Top
            20, 21, 22,     22, 23, 20      // Bottom
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Attribute 0: Position (3 floats)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Attribute 1: Normal (3 floats)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        // Attribute 2: TexCoord (2 floats)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void draw() const
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup()
    {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

// ============================================================================
// BASE PRIMITIVE: PROCEDURAL SPHERE MESH
// Created procedurally via latitude (stacks) and longitude (sectors) rings.
// Radius = 0.5 (diameter = 1.0) centered at (0, 0, 0).
// Normals are normalized vertex positions.
// ============================================================================
class SphereMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    SphereMesh(int stacks = 24, int sectors = 32, float radius = 0.5f) : VAO(0), VBO(0), EBO(0), indexCount(0)
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
        const float PI = 3.14159265358979323846f;

        for (int i = 0; i <= stacks; ++i)
        {
            float stackAngle = (PI / 2.0f) - (float)i * (PI / (float)stacks); // +pi/2 to -pi/2
            float xy = radius * cosf(stackAngle);
            float z = radius * sinf(stackAngle);

            for (int j = 0; j <= sectors; ++j)
            {
                float sectorAngle = (float)j * (2.0f * PI / (float)sectors); // 0 to 2pi

                float x = xy * cosf(sectorAngle);
                float y = xy * sinf(sectorAngle);

                // Position (swapping y and z so sphere pole is along Y axis)
                vertices.push_back(x);
                vertices.push_back(z); // Y in OpenGL is up
                vertices.push_back(y); // Z is depth

                // Normal (normalized)
                vertices.push_back(x / radius);
                vertices.push_back(z / radius);
                vertices.push_back(y / radius);

                // TexCoords
                vertices.push_back((float)j / (float)sectors);
                vertices.push_back((float)i / (float)stacks);
            }
        }

        // Indices
        for (int i = 0; i < stacks; ++i)
        {
            int k1 = i * (sectors + 1);
            int k2 = k1 + sectors + 1;

            for (int j = 0; j < sectors; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != (stacks - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        indexCount = (GLsizei)indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void draw() const
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup()
    {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

// ============================================================================
// BASE PRIMITIVE: SEAMLESS UMBRELLA CANOPY GORE MESH
// Represents one 30-degree curved gore sector (out of 12 sectors) of an umbrella canopy.
// Composed of 3 quad tiers (Apex->Mid, Mid->Rim, and Valance Flap) with dual-sided
// lighting normals. When 12 instances are placed in a circle, their side edges
// align with 100% mathematical precision with ZERO gaps and ZERO overlapping star artifacts!
// ============================================================================
class CanopyGoreMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    CanopyGoreMesh() : VAO(0), VBO(0), EBO(0), indexCount(36)
    {
        // Coordinates for one 30-degree sector (-15 deg to +15 deg)
        // sin(15) = 0.258819f, cos(15) = 0.965926f
        // Ring 0 (Apex):    R = 0.08, Y =  0.12  -> XL = -0.0207, XR = +0.0207, Z = 0.0773
        // Ring 1 (Mid):     R = 1.15, Y = -0.32  -> XL = -0.2976, XR = +0.2976, Z = 1.1108
        // Ring 2 (Rim):     R = 2.15, Y = -0.80  -> XL = -0.5565, XR = +0.5565, Z = 2.0767
        // Ring 3 (Valance): R = 2.15, Y = -0.95  -> XL = -0.5565, XR = +0.5565, Z = 2.0767

        // 16 vertices: 8 for top surface, 8 for underside (with inverted normals)
        GLfloat vertices[] = {
            // --- TOP SURFACE (Normals pointing UP/OUT) ---
            // 0: Apex Left
            -0.0207f,  0.12f, 0.0773f,    0.0f,  0.920f,  0.392f,   0.0f, 0.0f,
            // 1: Apex Right
             0.0207f,  0.12f, 0.0773f,    0.0f,  0.920f,  0.392f,   1.0f, 0.0f,
            // 2: Mid Left
            -0.2976f, -0.32f, 1.1108f,    0.0f,  0.901f,  0.433f,   0.0f, 0.5f,
            // 3: Mid Right
             0.2976f, -0.32f, 1.1108f,    0.0f,  0.901f,  0.433f,   1.0f, 0.5f,
            // 4: Rim Left
            -0.5565f, -0.80f, 2.0767f,    0.0f,  0.901f,  0.433f,   0.0f, 0.9f,
            // 5: Rim Right
             0.5565f, -0.80f, 2.0767f,    0.0f,  0.901f,  0.433f,   1.0f, 0.9f,
            // 6: Valance Left
            -0.5565f, -0.95f, 2.0767f,    0.0f,  0.000f,  1.000f,   0.0f, 1.0f,
            // 7: Valance Right
             0.5565f, -0.95f, 2.0767f,    0.0f,  0.000f,  1.000f,   1.0f, 1.0f,

            // --- UNDERSIDE SURFACE (Normals pointing DOWN/IN) ---
            // 8: Apex Left
            -0.0207f,  0.115f, 0.0773f,   0.0f, -0.920f, -0.392f,   0.0f, 0.0f,
            // 9: Apex Right
             0.0207f,  0.115f, 0.0773f,   0.0f, -0.920f, -0.392f,   1.0f, 0.0f,
            // 10: Mid Left
            -0.2976f, -0.325f, 1.1108f,   0.0f, -0.901f, -0.433f,   0.0f, 0.5f,
            // 11: Mid Right
             0.2976f, -0.325f, 1.1108f,   0.0f, -0.901f, -0.433f,   1.0f, 0.5f,
            // 12: Rim Left
            -0.5565f, -0.805f, 2.0767f,   0.0f, -0.901f, -0.433f,   0.0f, 0.9f,
            // 13: Rim Right
             0.5565f, -0.805f, 2.0767f,   0.0f, -0.901f, -0.433f,   1.0f, 0.9f,
            // 14: Valance Left
            -0.5565f, -0.955f, 2.0767f,   0.0f,  0.000f, -1.000f,   0.0f, 1.0f,
            // 15: Valance Right
             0.5565f, -0.955f, 2.0767f,   0.0f,  0.000f, -1.000f,   1.0f, 1.0f
        };

        GLuint indices[] = {
            // Top Surface
            0, 2, 1,    1, 2, 3,        // Tier 1: Apex to Mid
            2, 4, 3,    3, 4, 5,        // Tier 2: Mid to Rim
            4, 6, 5,    5, 6, 7,        // Tier 3: Valance Flap

            // Underside Surface (reversed winding)
            8, 9, 10,   9, 11, 10,      // Tier 1 Underside
            10, 11, 12, 11, 13, 12,     // Tier 2 Underside
            12, 13, 14, 13, 15, 14      // Tier 3 Underside
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void draw() const
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void cleanup()
    {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

#endif // MESH_PRIMITIVES_H
