#ifndef BEACH_DAY_SCENE_H
#define BEACH_DAY_SCENE_H

#include "Rendering.h"

// Scene geometry, simulation and camera. Original components are labelled below.

// ============================================================================
// SECTION: SceneEnvironment.h
// ============================================================================
#ifndef SCENE_ENVIRONMENT_H
#define SCENE_ENVIRONMENT_H
#include <cmath>

inline float g_sceneSeaLevel=0.0f;
inline float coastalTideLevel(double hours)
{
    // A gentle semidiurnal tide. Simulation hours, not wave seconds.
    return .22f*(float)sin(hours*6.283185307179586/12.42);
}
inline float tidalShoreInset(float level)
{
    return (level-.08f)/(level>.08f?.055f:.12f);
}
#endif

// ============================================================================
// SECTION: MeshPrimitives.h
// ============================================================================
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
            -0.55382f, -0.805f, 2.0667f,   0.0f, -0.901f, -0.433f,   0.0f, 0.9f,
            // 13: Rim Right
             0.55382f, -0.805f, 2.0667f,   0.0f, -0.901f, -0.433f,   1.0f, 0.9f,
            // 14: Valance Left
            -0.55382f, -0.955f, 2.0667f,   0.0f,  0.000f, -1.000f,   0.0f, 1.0f,
            // 15: Valance Right
             0.55382f, -0.955f, 2.0667f,   0.0f,  0.000f, -1.000f,   1.0f, 1.0f
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

inline float islandHash(float x, float z)
{
    float h = sinf(x * 127.1f + z * 311.7f) * 43758.5453f;
    return h - floorf(h);
}

inline float islandValueNoise(float x, float z)
{
    float ix = floorf(x), iz = floorf(z);
    float fx = x - ix, fz = z - iz;
    fx = fx * fx * (3.0f - 2.0f * fx);
    fz = fz * fz * (3.0f - 2.0f * fz);
    float a = glm::mix(islandHash(ix, iz), islandHash(ix + 1.0f, iz), fx);
    float b = glm::mix(islandHash(ix, iz + 1.0f), islandHash(ix + 1.0f, iz + 1.0f), fx);
    return glm::mix(a, b, fz);
}

inline float islandFBM(float x, float z)
{
    float value = 0.0f;
    float amplitude = 0.55f;
    for (int octave = 0; octave < 4; ++octave)
    {
        value += islandValueNoise(x, z) * amplitude;
        x = x * 2.03f + 13.7f;
        z = z * 2.03f - 9.1f;
        amplitude *= 0.48f;
    }
    return value;
}

// Signed distance to an irregular elongated island footprint. Positive values
// are dry land, zero is the shoreline, and negative values are surrounding sea.
inline float getIslandSignedDistance(float x, float z)
{
    const float centerZ = -100.0f;
    float nx = x / 110.0f;
    float nz = (z - centerZ) / 155.0f;
    float angle = atan2f(nz, nx);
    float outline = 1.0f
        + 0.110f * sinf(angle * 3.0f + 0.4f)
        + 0.065f * sinf(angle * 5.0f - 0.8f)
        + 0.045f * cosf(angle * 7.0f + 1.1f)
        + 0.085f * glm::max(cosf(angle - 0.10f), 0.0f)
        - 0.075f * powf(glm::max(cosf(angle + 1.0f), 0.0f), 4.0f);
    float radial = sqrtf(nx * nx + nz * nz);
    return (outline - radial) * 100.0f;
}

// Complete island cross-section: shallow seabed outside, continuous beach ring,
// then a gradual rise to an irregular central hill as drawn by the user.
inline float getCoastalTerrainElevation(float x, float z)
{
    float shoreDistance = getIslandSignedDistance(x, z);
    float broad = islandFBM(x * 0.035f + 9.0f, z * 0.035f - 4.0f);

    if (shoreDistance >= 0.0f)
    {
        float inland = glm::smoothstep(17.0f, 92.0f, shoreDistance);
        float nx = x / 110.0f;
        float nz = (z + 100.0f) / 155.0f;
        float mainPeak = expf(-(powf((nx + 0.10f) / 0.34f, 2.0f) + powf((nz + 0.04f) / 0.38f, 2.0f)));
        float eastPeak = expf(-(powf((nx - 0.38f) / 0.24f, 2.0f) + powf((nz - 0.18f) / 0.30f, 2.0f)));
        float northPeak = expf(-(powf((nx + 0.34f) / 0.28f, 2.0f) + powf((nz + 0.40f) / 0.25f, 2.0f)));
        float ridgeNoise = 1.0f - fabsf(2.0f * islandValueNoise(x * 0.048f, z * 0.048f) - 1.0f);
        ridgeNoise *= ridgeNoise;
        float hillMass = powf(inland, 1.15f) * (16.0f + broad * 10.0f + ridgeNoise * 5.5f);
        float summits = inland * (17.0f * mainPeak + 10.0f * eastPeak + 8.0f * northPeak);
        float erosion = (islandValueNoise(x * 0.13f, z * 0.13f) - 0.5f) * 1.8f * inland;
        return 0.08f + .055f * glm::min(shoreDistance,13.0f) + hillMass + summits + erosion;
    }

    float offshore = -shoreDistance;
    float shallowShelf = 0.08f - 0.12f * offshore;
    float deepening = -7.5f * glm::smoothstep(18.0f, 105.0f, offshore);
    float sandbar = 0.08f * expf(-powf((offshore - 10.0f) / 5.5f, 2.0f));
    return glm::max(shallowShelf + deepening + sandbar, -12.0f);
}

inline float getCoastalSeabedElevation(float z)
{
    return getCoastalTerrainElevation(0.0f, z);
}

inline float getIslandBackdropElevation(float x, float z)
{
    return getCoastalTerrainElevation(x, z);
}

// ============================================================================
// PROCEDURAL FRUSTUM / CYLINDER PRIMITIVE
// Unit height along Y. Different radii allow cylinders, tapered towers, and
// cones without approximating them with swollen spheres.
// ============================================================================
class FrustumMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    FrustumMesh(float bottomRadius = 0.5f, float topRadius = 0.5f, int segments = 32, bool addCaps = true)
        : VAO(0), VBO(0), EBO(0), indexCount(0)
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
        float slope = bottomRadius - topRadius;

        // Side wall uses duplicated seam vertices for stable UVs.
        for (int i = 0; i <= segments; ++i)
        {
            float u = (float)i / (float)segments;
            float a = u * 6.28318530718f;
            float c = cosf(a), s = sinf(a);
            glm::vec3 n = glm::normalize(glm::vec3(c, slope, s));
            float radii[2] = { bottomRadius, topRadius };
            float ys[2] = { -0.5f, 0.5f };
            for (int row = 0; row < 2; ++row)
            {
                vertices.insert(vertices.end(), {
                    radii[row] * c, ys[row], radii[row] * s,
                    n.x, n.y, n.z, u, (float)row
                });
            }
        }
        for (int i = 0; i < segments; ++i)
        {
            GLuint b = (GLuint)(i * 2);
            indices.insert(indices.end(), { b, b + 2, b + 1, b + 1, b + 2, b + 3 });
        }

        auto addCap = [&](float y, float radius, float normalY, bool reverse) {
            if (radius <= 0.0001f) return;
            GLuint center = (GLuint)(vertices.size() / 8);
            vertices.insert(vertices.end(), { 0.0f, y, 0.0f, 0.0f, normalY, 0.0f, 0.5f, 0.5f });
            for (int i = 0; i <= segments; ++i)
            {
                float a = ((float)i / (float)segments) * 6.28318530718f;
                float c = cosf(a), s = sinf(a);
                vertices.insert(vertices.end(), {
                    radius * c, y, radius * s, 0.0f, normalY, 0.0f,
                    0.5f + c * 0.5f, 0.5f + s * 0.5f
                });
            }
            for (int i = 0; i < segments; ++i)
            {
                if (reverse) indices.insert(indices.end(), { center, center + 2 + (GLuint)i, center + 1 + (GLuint)i });
                else         indices.insert(indices.end(), { center, center + 1 + (GLuint)i, center + 2 + (GLuint)i });
            }
        };
        if (addCaps)
        {
            addCap(-0.5f, bottomRadius, -1.0f, true);
            addCap( 0.5f, topRadius,     1.0f, false);
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
// BASE PRIMITIVE: PROCEDURAL TRIANGULAR PRISM MESH
// Unit dimensions along X, Y, Z centered at origin [-0.5, 0.5]^3.
// Cross section in XY plane:
// Apex at (0, +0.5, z), Bottom-left at (-0.5, -0.5, z), Bottom-right at (+0.5, -0.5, z).
// Extends from z = -0.5 to z = +0.5.
// Composed of 2 triangular caps (front & back) + 3 rectangular faces (bottom, left slope, right slope).
// Exact outward face normals for accurate diffuse and specular Phong lighting.
// ============================================================================
class PrismMesh
{
public:
    GLuint VAO, VBO;
    GLsizei vertexCount;

    PrismMesh() : VAO(0), VBO(0), vertexCount(24)
    {
        const float nRightX = 0.894427f;
        const float nRightY = 0.447214f;

        // 24 vertices (8 triangles * 3 vertices): posX, posY, posZ, normX, normY, normZ, u, v
        const GLfloat vertices[] = {
            // Front Triangle (Normal: 0, 0, 1)
            -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
             0.0f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.5f, 1.0f,

            // Back Triangle (Normal: 0, 0, -1)
             0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
             0.0f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.5f, 1.0f,

            // Bottom Face (Normal: 0, -1, 0)
            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

            // Right Slope Face (Normal: +nRightX, +nRightY, 0)
             0.5f, -0.5f,  0.5f,   nRightX, nRightY, 0.0f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   nRightX, nRightY, 0.0f,   1.0f, 0.0f,
             0.0f,  0.5f, -0.5f,   nRightX, nRightY, 0.0f,   1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,   nRightX, nRightY, 0.0f,   0.0f, 0.0f,
             0.0f,  0.5f, -0.5f,   nRightX, nRightY, 0.0f,   1.0f, 1.0f,
             0.0f,  0.5f,  0.5f,   nRightX, nRightY, 0.0f,   0.0f, 1.0f,

            // Left Slope Face (Normal: -nRightX, +nRightY, 0)
            -0.5f, -0.5f, -0.5f,  -nRightX, nRightY, 0.0f,   0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  -nRightX, nRightY, 0.0f,   1.0f, 0.0f,
             0.0f,  0.5f,  0.5f,  -nRightX, nRightY, 0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  -nRightX, nRightY, 0.0f,   0.0f, 0.0f,
             0.0f,  0.5f,  0.5f,  -nRightX, nRightY, 0.0f,   1.0f, 1.0f,
             0.0f,  0.5f, -0.5f,  -nRightX, nRightY, 0.0f,   0.0f, 1.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

    void cleanup()
    {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        VAO = 0;
        VBO = 0;
    }
};

// ============================================================================
// TRIANGULATED ISLAND BACKDROP
// A proper heightfield with computed normals. This is the hill geometry; it
// replaces the former collection of oversized ellipsoids.
// ============================================================================
class IslandTerrainMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    IslandTerrainMesh(float xMin = -145.0f, float xMax = 145.0f,
                      float zMin = -285.0f, float zMax = 82.0f,
                      int resX = 240, int resZ = 300)
        : VAO(0), VBO(0), EBO(0), indexCount(0)
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
        for (int iz = 0; iz <= resZ; ++iz)
        {
            float tz = (float)iz / (float)resZ;
            float z = glm::mix(zMin, zMax, tz);
            for (int ix = 0; ix <= resX; ++ix)
            {
                float tx = (float)ix / (float)resX;
                float x = glm::mix(xMin, xMax, tx);
                float y = getIslandBackdropElevation(x, z);
                float d = 0.35f;
                float yL = getIslandBackdropElevation(x - d, z);
                float yR = getIslandBackdropElevation(x + d, z);
                float yD = getIslandBackdropElevation(x, z - d);
                float yU = getIslandBackdropElevation(x, z + d);
                glm::vec3 n = glm::normalize(glm::vec3(yL - yR, 2.0f * d, yD - yU));
                vertices.insert(vertices.end(), { x, y, z, n.x, n.y, n.z, tx * 12.0f, tz * 7.0f });
            }
        }
        for (int iz = 0; iz < resZ; ++iz)
        {
            for (int ix = 0; ix < resX; ++ix)
            {
                GLuint a = (GLuint)(iz * (resX + 1) + ix);
                GLuint b = (GLuint)((iz + 1) * (resX + 1) + ix);
                indices.insert(indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
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
// BASE PRIMITIVE: WATER SURFACE GRID MESH
// Finely tessellated rectangular mesh driven by Gerstner wave displacement
// ============================================================================
class WaterGridMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    WaterGridMesh(float xMin = -2600.0f, float xMax = 2600.0f, float zMin = -2600.0f, float zMax = 2600.0f, int resX = 300, int resZ = 300)
        : VAO(0), VBO(0), EBO(0), indexCount(0)
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;

        for (int i = 0; i <= resZ; ++i)
        {
            float tz = (float)i / (float)resZ;
            // Non-linear spacing keeps several-metre tessellation around the
            // island while allowing the ocean to continue past the far clip.
            float uz = tz * 2.0f - 1.0f;
            float wz = 0.5f + 0.5f * (uz < 0.0f ? -1.0f : 1.0f) * powf(fabsf(uz), 1.60f);
            float z = glm::mix(zMin, zMax, wz);

            for (int j = 0; j <= resX; ++j)
            {
                float tx = (float)j / (float)resX;
                float ux = tx * 2.0f - 1.0f;
                float wx = 0.5f + 0.5f * (ux < 0.0f ? -1.0f : 1.0f) * powf(fabsf(ux), 1.60f);
                float x = glm::mix(xMin, xMax, wx);

                // Pos (x, 0, z)
                vertices.push_back(x);
                vertices.push_back(0.0f);
                vertices.push_back(z);

                // Normal (0, 1, 0)
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);

                // UV
                vertices.push_back(x * 0.05f);
                vertices.push_back(z * 0.05f);
            }
        }

        for (int i = 0; i < resZ; ++i)
        {
            for (int j = 0; j < resX; ++j)
            {
                int r1 = i * (resX + 1) + j;
                int r2 = (i + 1) * (resX + 1) + j;

                indices.push_back(r1);
                indices.push_back(r2);
                indices.push_back(r1 + 1);

                indices.push_back(r1 + 1);
                indices.push_back(r2);
                indices.push_back(r2 + 1);
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
// BASE PRIMITIVE: SLOPING SEABED & BEACH BATHYMETRY MESH (Diagram B)
// Continuous 3D undulating terrain spanning rear dunes down to deep ocean floor
// ============================================================================
class SlopingSeabedMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    SlopingSeabedMesh(float xMin = -600.0f, float xMax = 600.0f, float zMin = -120.0f, float zMax = 400.0f, int resX = 200, int resZ = 140)
        : VAO(0), VBO(0), EBO(0), indexCount(0)
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;

        for (int i = 0; i <= resZ; ++i)
        {
            float tz = (float)i / (float)resZ;
            float z = glm::mix(zMin, zMax, tz);

            for (int j = 0; j <= resX; ++j)
            {
                float tx = (float)j / (float)resX;
                float x = glm::mix(xMin, xMax, tx);
                float y = getCoastalTerrainElevation(x, z);

                // Exact 2D central finite-difference normal for realistic surface lighting
                float dx = 0.5f;
                float dz = 0.5f;
                float yL = getCoastalTerrainElevation(x - dx, z);
                float yR = getCoastalTerrainElevation(x + dx, z);
                float yD = getCoastalTerrainElevation(x, z - dz);
                float yU = getCoastalTerrainElevation(x, z + dz);
                glm::vec3 norm = glm::normalize(glm::vec3(-(yR - yL) / (2.0f * dx), 1.0f, -(yU - yD) / (2.0f * dz)));

                // Pos
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                // Normal
                vertices.push_back(norm.x);
                vertices.push_back(norm.y);
                vertices.push_back(norm.z);

                // UV
                vertices.push_back(x * 0.05f);
                vertices.push_back(z * 0.05f);
            }
        }

        for (int i = 0; i < resZ; ++i)
        {
            for (int j = 0; j < resX; ++j)
            {
                int r1 = i * (resX + 1) + j;
                int r2 = (i + 1) * (resX + 1) + j;

                indices.push_back(r1);
                indices.push_back(r2);
                indices.push_back(r1 + 1);

                indices.push_back(r1 + 1);
                indices.push_back(r2);
                indices.push_back(r2 + 1);
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
// BASE PRIMITIVE: 3D WATER DEPTH CROSS-SECTION SKIRT MESH (Diagram A)
// Vertical cutout volume walls on left, right, and offshore edges illustrating
// water depth volume and subterranean bathymetric profile.
// ============================================================================
class WaterDepthSkirtMesh
{
public:
    GLuint VAO, VBO, EBO;
    GLsizei indexCount;

    WaterDepthSkirtMesh(float xMin = -260.0f, float xMax = 260.0f, float zMin = 0.0f, float zMax = 250.0f, int stepsZ = 90, int stepsX = 80)
        : VAO(0), VBO(0), EBO(0), indexCount(0)
    {
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;

        auto addQuad = [&](const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec3& norm) {
            GLuint base = (GLuint)(vertices.size() / 8);
            glm::vec3 pts[4] = { p1, p2, p3, p4 };
            for (int k = 0; k < 4; ++k)
            {
                vertices.push_back(pts[k].x);
                vertices.push_back(pts[k].y);
                vertices.push_back(pts[k].z);
                vertices.push_back(norm.x);
                vertices.push_back(norm.y);
                vertices.push_back(norm.z);
                vertices.push_back((k == 1 || k == 2) ? 1.0f : 0.0f);
                vertices.push_back((k >= 2) ? 1.0f : 0.0f);
            }
            indices.push_back(base);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        };

        // 1. Left side wall (at xMin from zMin to zMax)
        for (int i = 0; i < stepsZ; ++i)
        {
            float z1 = glm::mix(zMin, zMax, (float)i / (float)stepsZ);
            float z2 = glm::mix(zMin, zMax, (float)(i + 1) / (float)stepsZ);
            float yBottom1 = getCoastalTerrainElevation(xMin, z1);
            float yBottom2 = getCoastalTerrainElevation(xMin, z2);

            addQuad(
                glm::vec3(xMin, 0.0f, z1),
                glm::vec3(xMin, 0.0f, z2),
                glm::vec3(xMin, yBottom2, z2),
                glm::vec3(xMin, yBottom1, z1),
                glm::vec3(-1.0f, 0.0f, 0.0f)
            );
        }

        // 2. Right side wall (at xMax from zMin to zMax)
        for (int i = 0; i < stepsZ; ++i)
        {
            float z1 = glm::mix(zMin, zMax, (float)i / (float)stepsZ);
            float z2 = glm::mix(zMin, zMax, (float)(i + 1) / (float)stepsZ);
            float yBottom1 = getCoastalTerrainElevation(xMax, z1);
            float yBottom2 = getCoastalTerrainElevation(xMax, z2);

            addQuad(
                glm::vec3(xMax, 0.0f, z2),
                glm::vec3(xMax, 0.0f, z1),
                glm::vec3(xMax, yBottom1, z1),
                glm::vec3(xMax, yBottom2, z2),
                glm::vec3(1.0f, 0.0f, 0.0f)
            );
        }

        // 3. Deep offshore back wall (at zMax from xMin to xMax)
        for (int j = 0; j < stepsX; ++j)
        {
            float x1 = glm::mix(xMin, xMax, (float)j / (float)stepsX);
            float x2 = glm::mix(xMin, xMax, (float)(j + 1) / (float)stepsX);
            float yBottom1 = getCoastalTerrainElevation(x1, zMax);
            float yBottom2 = getCoastalTerrainElevation(x2, zMax);

            addQuad(
                glm::vec3(x1, 0.0f, zMax),
                glm::vec3(x1, yBottom1, zMax),
                glm::vec3(x2, yBottom2, zMax),
                glm::vec3(x2, 0.0f, zMax),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
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

#endif // MESH_PRIMITIVES_H

// ============================================================================
// SECTION: BoatHull.h
// ============================================================================
#ifndef BOAT_HULL_H
#define BOAT_HULL_H

#include <glm/gtc/type_ptr.hpp>

// Lofted hull stations: fine bow entry, broad flat transom, flared topsides
// and a hard chine. The origin is the design waterline, bow along local +Z.
class BoatHullMesh
{
    GLuint vao = 0, vbo = 0;
    struct Part { GLint first; GLsizei count; glm::vec3 color; float gloss; };
    std::vector<Part> parts;
public:
    explicit BoatHullMesh(bool sailboat)
    {
        std::vector<float> vertices;
        const int stations = 80;
        const float stern = sailboat ? -1.74f : -1.97f;
        const float bow = sailboat ? 1.95f : 2.25f;
        auto width = [sailboat](float t) {
            return (sailboat ? 0.62f : 0.73f) * (1.0f - powf(t, 3.4f)) + 0.018f;
        };
        auto position = [&](float t, float u, int side) {
            float w = width(t);
            float entry = powf(t, 7.0f);
            const float xs[] = {0.0f, 0.80f, 0.96f, 0.97f, 0.985f, 1.0f, 1.0f};
            const float ys[] = {-0.32f, -0.075f, 0.10f, 0.13f, 0.18f, 0.45f, 0.49f};
            int band = glm::min((int)u, 5);
            float f = u - (float)band;
            float y = glm::mix(ys[band], ys[band + 1], f);
            y += entry * ((u < 2.0f) ? 0.40f : 0.12f);
            return glm::vec3(side * w * glm::mix(xs[band], xs[band + 1], f),
                             y, glm::mix(stern, bow, t));
        };
        auto emit = [&](glm::vec3 p, glm::vec3 n, float t, float u) {
            vertices.insert(vertices.end(), {p.x,p.y,p.z,n.x,n.y,n.z,t,u});
        };
        auto triangle = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n) {
            emit(a,n,0,0); emit(b,n,0,0); emit(c,n,0,0);
        };
        const glm::vec3 navy = sailboat ? glm::vec3(0.07f,0.16f,0.29f) : glm::vec3(0.055f,0.13f,0.23f);
        const glm::vec3 colors[] = {navy * 0.65f, navy, navy,
            glm::vec3(0.72f,0.055f,0.035f), glm::vec3(0.88f,0.91f,0.93f), glm::vec3(0.065f,0.075f,0.085f)};
        for (int band = 0; band < 6; ++band)
        {
            GLint first = (GLint)vertices.size() / 8;
            for (int side : {-1, 1}) for (int i = 0; i < stations; ++i)
            {
                float t0 = (float)i / stations, t1 = (float)(i + 1) / stations;
                auto vertex = [&](float t, float u) {
                    glm::vec3 dz = position(glm::min(t + 0.001f,1.0f),u,side)
                                 - position(glm::max(t - 0.001f,0.0f),u,side);
                    glm::vec3 du = position(t,(float)band+1,side)-position(t,(float)band,side);
                    emit(position(t,u,side),glm::normalize(glm::cross(du,dz)*(float)side),t,u/6.0f);
                };
                vertex(t0,(float)band); vertex(t1,(float)band); vertex(t0,(float)band+1);
                vertex(t0,(float)band+1); vertex(t1,(float)band); vertex(t1,(float)band+1);
            }
            parts.push_back({first,(GLsizei)vertices.size()/8-first,colors[band],band==5?24.0f:90.0f});
        }
        GLint first = (GLint)vertices.size()/8;
        // Close the transom; no ellipsoids or disconnected rectangular bow.
        for (int side : {-1,1}) for (int band=0;band<6;++band)
        {
            glm::vec3 a=position(0,(float)band,side), b=position(0,(float)band+1,side);
            triangle(glm::vec3(0,a.y,stern),a,b,glm::vec3(0,0,-1));
            triangle(glm::vec3(0,a.y,stern),b,glm::vec3(0,b.y,stern),glm::vec3(0,0,-1));
        }
        parts.push_back({first,(GLsizei)vertices.size()/8-first,colors[4],75.0f});
        first=(GLint)vertices.size()/8;
        for (int i=0;i<stations;++i)
        {
            float t0=(float)i/stations,t1=(float)(i+1)/stations;
            // Open motorboat cockpit, covered foredeck and narrow aft deck.
            if (!sailboat && t0>0.11f && t0<0.65f) continue;
            glm::vec3 a=position(t0,5,-1),b=position(t0,5,1);
            glm::vec3 c=position(t1,5,-1),d=position(t1,5,1);
            triangle(a,c,b,glm::vec3(0,1,0)); triangle(b,c,d,glm::vec3(0,1,0));
        }
        parts.push_back({first,(GLsizei)vertices.size()/8-first,
                        sailboat?glm::vec3(0.65f,0.47f,0.28f):colors[4],35.0f});
        // Inboard lining gives the cockpit rim thickness and readable depth.
        first=(GLint)vertices.size()/8;
        for(int side:{-1,1}) for(int i=9;i<53;++i)
        {
            float t0=(float)i/stations,t1=(float)(i+1)/stations;
            glm::vec3 a=position(t0,5,side),b=position(t1,5,side);
            a.x-=side*0.045f; b.x-=side*0.045f;
            glm::vec3 c=a,d=b; c.y=0.18f; d.y=0.18f;
            triangle(a,b,c,glm::vec3((float)-side,0,0));
            triangle(c,b,d,glm::vec3((float)-side,0,0));
        }
        if (!sailboat) parts.push_back({first,(GLsizei)vertices.size()/8-first,colors[4],40.0f});
        glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
        glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
        for(int attr=0;attr<3;++attr)
        {
            glVertexAttribPointer(attr,attr==2?2:3,GL_FLOAT,GL_FALSE,8*sizeof(float),
                                  (void*)((attr==0?0:attr==1?3:6)*sizeof(float)));
            glEnableVertexAttribArray(attr);
        }
        glBindVertexArray(0);
    }
    void draw(Shader& shader,const glm::mat4& transform) const
    {
        glUniformMatrix4fv(shader.location("model"),1,GL_FALSE,glm::value_ptr(transform));
        glUniform1i(shader.location("boatHullPass"),1);
        glUniform1f(shader.location("boatWaterHeight"),transform[3].y);
        glUniform1i(shader.location("useTexture"),0);
        glUniform1i(shader.location("shadingMode"),0);
        glUniform1i(shader.location("unlit"),0);
        glUniform1f(shader.location("specularStrength"),0.45f);
        glBindVertexArray(vao);
        for(const Part& p:parts)
        {
            glUniform3fv(shader.location("objectColor"),1,glm::value_ptr(p.color));
            glUniform1f(shader.location("shininess"),p.gloss);
            glDrawArrays(GL_TRIANGLES,p.first,p.count);
        }
        glBindVertexArray(0);
        glUniform1i(shader.location("boatHullPass"),0);
    }
    void cleanup() { glDeleteBuffers(1,&vbo); glDeleteVertexArrays(1,&vao); vbo=vao=0; }
};

inline BoatHullMesh& boatHull(bool sailboat)
{
    static BoatHullMesh motor(false), sail(true);
    return sailboat?sail:motor;
}

class BoatSailMesh
{
    GLuint vao=0,vbo=0;
    GLsizei count=0;
public:
    explicit BoatSailMesh(bool jib)
    {
        glm::vec3 base(jib?-0.02f:0.02f,jib?0.60f:0.90f,jib?0.43f:0.40f);
        glm::vec3 up(0,jib?2.0f:2.56f,0),aft(0,0,jib?1.47f:-1.55f);
        std::vector<float> data;
        const int resolution=24;
        auto emit=[&](int i,int j) {
            float u=(float)i/resolution,v=(float)j/resolution;
            glm::vec3 p=base+up*u+aft*v;
            p.x+=4.0f*u*v*(1-u-v); // Smooth wind-filled camber, zero at all edges.
            glm::vec3 du=up+glm::vec3(4*v*(1-2*u-v),0,0);
            glm::vec3 dv=aft+glm::vec3(4*u*(1-u-2*v),0,0);
            glm::vec3 n=glm::normalize(glm::cross(du,dv));
            data.insert(data.end(),{p.x,p.y,p.z,n.x,n.y,n.z,u,v});
        };
        for(int i=0;i<resolution;++i) for(int j=0;j<resolution-i;++j)
        {
            emit(i,j);emit(i+1,j);emit(i,j+1);
            if(i+j<resolution-1) {emit(i+1,j);emit(i+1,j+1);emit(i,j+1);}
        }
        count=(GLsizei)data.size()/8;
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(float),data.data(),GL_STATIC_DRAW);
        for(int attr=0;attr<3;++attr)
        {
            glVertexAttribPointer(attr,attr==2?2:3,GL_FLOAT,GL_FALSE,8*sizeof(float),
                (void*)((attr==0?0:attr==1?3:6)*sizeof(float)));
            glEnableVertexAttribArray(attr);
        }
        glBindVertexArray(0);
    }
    void draw(Shader& shader,const glm::mat4& transform) const
    {
        glUniformMatrix4fv(shader.location("model"),1,GL_FALSE,glm::value_ptr(transform));
        glUniform3f(shader.location("objectColor"),0.90f,0.89f,0.82f);
        glUniform1f(shader.location("specularStrength"),0.05f);
        glUniform1f(shader.location("shininess"),12);
        glUniform1i(shader.location("boatSailPass"),1);
        glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,count);glBindVertexArray(0);
        glUniform1i(shader.location("boatSailPass"),0);
    }
    void cleanup(){glDeleteBuffers(1,&vbo);glDeleteVertexArrays(1,&vao);vbo=vao=0;}
};

inline BoatSailMesh& boatSail(bool jib)
{
    static BoatSailMesh main(false), foresail(true);
    return jib?foresail:main;
}

#endif

// ============================================================================
// SECTION: ATVPanel.h
// ============================================================================
#ifndef ATV_PANEL_H
#define ATV_PANEL_H

#include <glm/gtc/type_ptr.hpp>

// Lofted, bevelled plastic shell: broad crown, sloping nose, rolled edges.
// Reused for the hood and mudguards rather than stacking rectangular blocks.
class ATVPanelMesh
{
    GLuint vao=0,vbo=0;
    GLsizei count=0;
public:
    ATVPanelMesh()
    {
        std::vector<float> vertices;
        const float z[]={-.5f,-.35f,.20f,.40f,.5f};
        const float width[]={.36f,.5f,.5f,.44f,.31f};
        const float top[]={.1f,.45f,.5f,.28f,-.10f};
        auto point=[&](int station,int edge) {
            const float x[]={-.78f,.78f,1,1,.78f,-.78f,-1,-1};
            float y[]={top[station],top[station],top[station]-.18f,-.32f,-.5f,-.5f,-.32f,top[station]-.18f};
            return glm::vec3(x[edge]*width[station],y[edge],z[station]);
        };
        auto triangle=[&](glm::vec3 a,glm::vec3 b,glm::vec3 c) {
            glm::vec3 n=glm::normalize(glm::cross(b-a,c-a));
            for(glm::vec3 p:{a,b,c})vertices.insert(vertices.end(),{p.x,p.y,p.z,n.x,n.y,n.z,p.x+.5f,p.z+.5f});
        };
        for(int s=0;s<4;++s)for(int e=0;e<8;++e)
        {
            int next=(e+1)%8;
            glm::vec3 a=point(s,e),b=point(s,next),c=point(s+1,e),d=point(s+1,next);
            triangle(a,c,b);triangle(b,c,d);
        }
        for(int e=1;e<7;++e)
        {
            triangle(point(0,0),point(0,e),point(0,e+1));
            triangle(point(4,0),point(4,e+1),point(4,e));
        }
        count=(GLsizei)vertices.size()/8;
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);
        glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
        for(int attr=0;attr<3;++attr)
        {
            glVertexAttribPointer(attr,attr==2?2:3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)((attr==0?0:attr==1?3:6)*sizeof(float)));
            glEnableVertexAttribArray(attr);
        }
        glBindVertexArray(0);
    }
    void draw(Shader& shader,const glm::mat4& m,const glm::vec3& color) const
    {
        glUniformMatrix4fv(shader.location("model"),1,GL_FALSE,glm::value_ptr(m));
        glUniform3fv(shader.location("objectColor"),1,glm::value_ptr(color));
        glUniform1f(shader.location("shininess"),48);
        glUniform1f(shader.location("specularStrength"),.25f);
        glUniform1i(shader.location("unlit"),0);
        glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,count);glBindVertexArray(0);
    }
};

inline const ATVPanelMesh& atvPanel()
{
    static ATVPanelMesh mesh;
    return mesh;
}
#endif

// ============================================================================
// SECTION: SceneObjects.h
// ============================================================================
#ifndef SCENE_OBJECTS_H
#define SCENE_OBJECTS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

// Primitive capture keeps the existing geometry and material definitions as the
// single source of truth. Only animated palm transforms are refreshed per frame.
struct ForestInstance {
    glm::mat4 model;
    glm::vec3 color,material;
};
struct ForestCapture {
    std::vector<ForestInstance> cubes,leaves,spheres;
    std::vector<glm::vec3> roots;
    bool textured=false;
    void add(bool sphere,const glm::mat4& model,glm::vec3 color,float gloss,float spec,int unlit) {
        (sphere?spheres:textured?leaves:cubes).push_back({model,color,{gloss,spec,(float)unlit}});
    }
};
inline ForestCapture* g_forestCapture=nullptr;

inline void drawTransformedCube(
    Shader& shader,
    const CubeMesh& cube,
    const glm::mat4& model,
    const glm::vec3& color,
    float shininess = 32.0f,
    float specularStrength = 0.25f,
    int unlit = 0)
{
    if(g_forestCapture){g_forestCapture->add(false,model,color,shininess,specularStrength,unlit);return;}
    GLuint modelLoc = shader.location("model");
    GLuint colorLoc = shader.location("objectColor");
    GLuint specLoc  = shader.location("specularStrength");
    GLuint shinLoc  = shader.location("shininess");
    GLuint unlitLoc = shader.location("unlit");

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
    if(g_forestCapture){g_forestCapture->add(true,model,color,shininess,specularStrength,unlit);return;}
    GLuint modelLoc = shader.location("model");
    GLuint colorLoc = shader.location("objectColor");
    GLuint specLoc  = shader.location("specularStrength");
    GLuint shinLoc  = shader.location("shininess");
    GLuint unlitLoc = shader.location("unlit");

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
    glUniformMatrix4fv(shader.location("model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(shader.location("objectColor"), 1, glm::value_ptr(color));
    glUniform1f(shader.location("specularStrength"), specularStrength);
    glUniform1f(shader.location("shininess"), shininess);
    glUniform1i(shader.location("unlit"), unlit);
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
    GLuint modelLoc = shader.location("model");
    GLuint colorLoc = shader.location("objectColor");
    GLuint specLoc  = shader.location("specularStrength");
    GLuint shinLoc  = shader.location("shininess");
    GLuint unlitLoc = shader.location("unlit");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    glUniform1f(specLoc, specularStrength);
    glUniform1f(shinLoc, shininess);
    glUniform1i(unlitLoc, unlit);

    gore.draw();
}

inline void drawTransformedPrism(
    Shader& shader,
    const PrismMesh& prism,
    const glm::mat4& model,
    const glm::vec3& color,
    float shininess = 16.0f,
    float specularStrength = 0.15f,
    int unlit = 0)
{
    glUniformMatrix4fv(shader.location("model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(shader.location("objectColor"), 1, glm::value_ptr(color));
    glUniform1f(shader.location("specularStrength"), specularStrength);
    glUniform1f(shader.location("shininess"), shininess);
    glUniform1i(shader.location("unlit"), unlit);
    prism.draw();
}

// ============================================================================
// OBJECT 1: ROBOT (SEATED UNDER UMBRELLA)
// Constructed hierarchically with cubes and spheres.
// Fully editable parameters: head turn, arm gestures, leg seating angles.
// ============================================================================
struct RobotLegPose {
    float hipPitch[2]={0,0},kneePitch[2]={0,0},anklePitch[2]={0,0};
    float shinLength=.39f,hipDrop=.07f;
};

inline void drawRobot(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform,
    float headYaw = 10.0f,
    float headPitch = 0.0f,
    float leftArmPitch = -25.0f,
    float rightArmPitch = 20.0f,
    float legBendAngle = 80.0f,
    float hipSpread = 0.16f,
    const RobotLegPose* legPose = nullptr)
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
        glm::mat4 hipMat = glm::translate(baseTransform, glm::vec3(side * hipSpread, legPose?-legPose->hipDrop:-.07f, 0.04f));
        glm::mat4 hipBall = glm::scale(hipMat, glm::vec3(0.14f, 0.14f, 0.14f));
        drawTransformedSphere(shader, sphere, hipBall, darkJointColor);

        // Thigh (Rotated 80 degrees forward to sit on chair)
        int index=side==-1?0:1;
        float hipPitch=legPose?legPose->hipPitch[index]:legBendAngle;
        float kneePitch=legPose?legPose->kneePitch[index]:-legBendAngle;
        float shinLength=legPose?legPose->shinLength:.39f;
        glm::mat4 thighMat = glm::rotate(hipMat, glm::radians(hipPitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 thighSeg = glm::translate(thighMat, glm::vec3(0.0f, -0.20f, 0.0f));
        glm::mat4 thighScale = glm::scale(thighSeg, glm::vec3(0.14f, 0.32f, 0.14f));
        drawTransformedCube(shader, cube, thighScale, metalBodyColor);

        // Knee Joint
        glm::mat4 kneeMat = glm::translate(thighMat, glm::vec3(0.0f, -0.39f, 0.0f));
        glm::mat4 kneeBall = glm::scale(kneeMat, glm::vec3(0.13f, 0.13f, 0.13f));
        drawTransformedSphere(shader, sphere, kneeBall, darkJointColor);

        // Shin / Lower Leg (Bent down 80 degrees relative to thigh)
        glm::mat4 shinMat = glm::rotate(kneeMat, glm::radians(kneePitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 shinSeg = glm::translate(shinMat, glm::vec3(0.0f, legPose?-shinLength*.5f:-.20f, 0.0f));
        glm::mat4 shinScale = glm::scale(shinSeg, glm::vec3(0.13f, shinLength-.07f, 0.13f));
        drawTransformedCube(shader, cube, shinScale, metalBodyColor);

        // Ankle / Foot
        glm::mat4 footMat = glm::translate(shinMat, glm::vec3(0.0f, -shinLength, 0));
        if(legPose)footMat=glm::rotate(footMat,glm::radians(legPose->anklePitch[index]),glm::vec3(1,0,0));
        footMat=glm::translate(footMat,glm::vec3(0,0,.09f));
        glm::mat4 footScale = glm::scale(footMat, glm::vec3(0.17f, 0.10f, 0.28f));
        drawTransformedCube(shader, cube, footScale, darkJointColor);
    }
}

// ============================================================================
// OBJECT 2: BEACH UMBRELLA
// Hierarchical modeling: Tilting the pole automatically tilts the canopy!
// Canopy is crafted from alternating coral red and white gore panels.
// ============================================================================
struct UmbrellaAnimationState {
    bool targetOpen=true,keyDown=false;
    float openAmount=1,elapsed=0,startAmount=1;
    static constexpr float duration=1.5f;
    bool press(bool down) {
        bool toggled=down && !keyDown;keyDown=down;
        if(toggled){targetOpen=!targetOpen;startAmount=openAmount;elapsed=0;}
        return toggled;
    }
    void update(float dt) {
        if(!std::isfinite(dt) || dt<=0)return;
        elapsed=glm::min(duration,elapsed+dt);
        if(duration-elapsed<.00001f)elapsed=duration;
        float t=elapsed/duration;t=t*t*(3-2*t);
        openAmount=glm::clamp(glm::mix(startAmount,targetOpen?1.f:0.f,t),0.f,1.f);
    }
    const char* label()const {
        if(targetOpen && openAmount>=1)return "OPEN";
        if(!targetOpen && openAmount<=0)return "CLOSED";
        return targetOpen?"OPENING":"CLOSING";
    }
};

// Hinge at the existing apex: positive X rotation lowers a radial (+Z) rib.
// Width narrows as fabric gathers; Y is never compressed, retaining the
// original top/underside separation and valance thickness.
inline glm::mat4 umbrellaPanelTransform(float opening,int sector) {
    float fold=1-glm::clamp(opening,0.f,1.f);
    glm::mat4 panel=glm::rotate(glm::mat4(1),glm::radians(sector*30.f),glm::vec3(0,1,0));
    if(fold==0)return panel; // Preserve the fully-open mesh exactly.
    panel=glm::translate(panel,glm::vec3(0,.12f,.0773f));
    panel=glm::rotate(panel,glm::radians(60.f*fold),glm::vec3(1,0,0));
    // Gather the cloth radially before lowering it past a seated occupant.
    // Return to full hanging length at the closed endpoint.
    float radialGather=1-.25f*sinf(3.14159265f*fold);
    panel=glm::scale(panel,glm::vec3(glm::mix(1.f,.10f,fold),1,radialGather));
    return glm::translate(panel,glm::vec3(0,-.12f,-.0773f));
}

inline void drawUmbrella(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const CanopyGoreMesh& gore,
    const glm::mat4& baseTransform,
    float poleTiltAngle = 12.0f,
    float poleHeight = 3.2f,
    float canopyScale = 1.0f,float opening=1.0f)
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
    GLboolean previouslyCulled=glIsEnabled(GL_CULL_FACE);
    GLint previousCullMode,previousWinding;
    glGetIntegerv(GL_CULL_FACE_MODE,&previousCullMode);
    glGetIntegerv(GL_FRONT_FACE,&previousWinding);
    glEnable(GL_CULL_FACE);glCullFace(GL_BACK);glFrontFace(GL_CCW);
    for (int i = 0; i < 12; ++i)
    {
        glm::vec3 panelColor = (i % 2 == 0) ? stripeRed : stripeWhite;

        glm::mat4 goreMat = canopyCrown*umbrellaPanelTransform(opening,i);
        drawTransformedGore(shader, gore, goreMat, panelColor);
        // Two short rectangular ribs follow each original gore's underside.
        // Ends are inset slightly so steel never meets an adjacent panel.
        const glm::vec3 ribPoints[]={{0,.10f,.09f},{0,-.34f,1.11f},{0,-.82f,2.05f}};
        for(int segment=0;segment<2;++segment){
            glm::vec3 a=ribPoints[segment],b=ribPoints[segment+1],d=b-a;
            glm::mat4 rib=glm::translate(goreMat,(a+b)*.5f);
            rib=glm::rotate(rib,atan2f(-d.y,d.z),glm::vec3(1,0,0));
            drawTransformedCube(shader,cube,glm::scale(rib,glm::vec3(.022f,.022f,glm::length(d))),ribColor,24,.18f);
        }
    }
    glCullFace(previousCullMode);glFrontFace(previousWinding);
    if(!previouslyCulled)glDisable(GL_CULL_FACE);

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

    // Continuous lofted displacement hull, curved deck edge and waterline bands.
    boatHull(true).draw(shader, baseTransform);

    // Bowsprit tip
    glm::mat4 bowTip = glm::translate(baseTransform, glm::vec3(0.0f, 0.34f, 2.05f));
    bowTip = glm::scale(bowTip, glm::vec3(0.055f, 0.055f, 0.52f));
    drawTransformedCube(shader, cube, bowTip, mastBrown);

    // Deck and transom are part of the shaped hull mesh.

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
        glUniform1i(shader.location("useTexture"), 1);
        glUniform1f(shader.location("texScale"), 1.5f);
    }

    boatSail(false).draw(shader,baseTransform);
    boatSail(true).draw(shader,baseTransform);

    if (texMgr && enableTextures)
    {
        glUniform1i(shader.location("useTexture"), 0);
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
    // Single fair hull with a fine entry, V bottom, flared sides and cockpit rim.
    boatHull(false).draw(shader, baseTransform);

    // Motor splashwell cutout recess at center transom
    glm::mat4 splashwellMat = glm::translate(baseTransform, glm::vec3(0.0f, 0.14f, -1.74f));
    splashwellMat = glm::scale(splashwellMat, glm::vec3(0.56f, 0.22f, 0.30f));
    drawTransformedCube(shader, cube, splashwellMat, rubRailColor, 32.0f, 0.20f);

    // Rub rail follows the lofted sheer rather than straight floating panels.

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
    // Foredeck is integrated into the hull and tapers to the bow.

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
        float driverHeadYaw = rudderAngle * 0.85f;
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
    enginePivot = glm::rotate(enginePivot, glm::radians(-rudderAngle), glm::vec3(0.0f, 1.0f, 0.0f));
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

    // Wake and bow foam are evaluated on the displaced water surface.
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
    if(g_forestCapture)g_forestCapture->roots.emplace_back(baseTransform[3]);
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
                    if(g_forestCapture)g_forestCapture->textured=true;
                    if (texMgr && enableTextures && !g_forestCapture)
                    {
                        texMgr->bind(TEX_PALM);
                        glUniform1i(shader.location("useTexture"), 1);
                        glUniform1f(shader.location("texScale"), 1.0f);
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

                    if (texMgr && enableTextures && !g_forestCapture)
                    {
                        glUniform1i(shader.location("useTexture"), 0);
                        texMgr->unbind();
                    }
                }

                if(g_forestCapture)g_forestCapture->textured=false;
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
        glUniform1i(shader.location("useTexture"), 1);
        glUniform1f(shader.location("texScale"), 36.0f);
    }
    // GOURAUD SHADING for expansive sea water surface
    glUniform1i(shader.location("shadingMode"), 1);

    glm::mat4 seaMat = glm::translate(glm::mat4(1.0f), seaCenter);
    seaMat = glm::scale(seaMat, glm::vec3(width, 0.2f, depth));

    // Water has high shininess and specular highlight for sun reflection
    drawTransformedCube(shader, cube, seaMat, waterColor, 128.0f, 0.85f, 0);

    if (texMgr && enableTextures)
    {
        glUniform1i(shader.location("useTexture"), 0);
        texMgr->unbind();
    }

    // Restore to default Phong
    glUniform1i(shader.location("shadingMode"), 0);
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

    float outsideDistance = glm::max(tidalShoreInset(g_sceneSeaLevel)-getIslandSignedDistance(x, z), 0.0f);
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

    outPos = glm::vec3(x + displacement.x, g_sceneSeaLevel+displacement.y, z + displacement.z);
    outNormal = glm::normalize(normalAccum);
}

// Gerstner displaces X/Z as well as height. Invert that displacement so
// buoyancy samples the visible surface at the requested world coordinates.
inline void getWaterSurfaceAt(float x, float z, float time,
                              glm::vec3& outPos, glm::vec3& outNormal)
{
    glm::vec2 base(x,z);
    for (int i=0;i<4;++i)
    {
        getGerstnerWaveAt(base.x,base.y,time,outPos,outNormal);
        base += glm::vec2(x-outPos.x,z-outPos.z);
    }
    getGerstnerWaveAt(base.x,base.y,time,outPos,outNormal);
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
    float boatSpeed = 0.0f,
    const glm::vec4& sailboat2 = glm::vec4(0.0f),
    const glm::vec4& sailboat3 = glm::vec4(0.0f),
    const std::vector<glm::vec4>& wakeTrail = {},
    const std::vector<glm::mat4>& hullTransforms = {})
{
    waterShader.Activate();

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(waterShader.location("model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(waterShader.location("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(waterShader.location("proj"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(waterShader.location("viewPos"), 1, glm::value_ptr(camPos));
    glUniform3fv(waterShader.location("lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(waterShader.location("lightColor"), 1, glm::value_ptr(lightColor));
    glUniform1f(waterShader.location("ambientStrength"), ambientStrength);
    glUniform1f(waterShader.location("time"), time);
    glUniform1f(waterShader.location("seaLevel"),g_sceneSeaLevel);

    // Boat interaction uniforms for dynamic wake and bow wave
    glUniform3fv(waterShader.location("boatPos"), 1, glm::value_ptr(boatPos));
    glUniform1f(waterShader.location("boatHeading"), boatHeading);
    glUniform1f(waterShader.location("boatSpeed"), boatSpeed);
    const glm::vec4 hulls[] = {
        glm::vec4(boatPos.x,boatPos.z,boatHeading,1.65f), sailboat2, sailboat3
    };
    glUniform4fv(waterShader.location("hulls[0]"),3,glm::value_ptr(hulls[0]));
    // Use the exact buoyant pose, including pitch/roll, to exclude water only
    // from the enclosed hull volume. Flat poses keep older callers compatible.
    glm::mat4 inverseHulls[3];
    for(int i=0;i<3;++i)
    {
        glm::mat4 pose=glm::translate(glm::mat4(1),glm::vec3(hulls[i].x,boatPos.y,hulls[i].y));
        pose=glm::rotate(pose,glm::radians(hulls[i].z),glm::vec3(0,1,0));
        pose=glm::scale(pose,glm::vec3(glm::max(hulls[i].w,.001f)));
        if(i<(int)hullTransforms.size())pose=hullTransforms[i];
        inverseHulls[i]=glm::inverse(pose);
    }
    glUniformMatrix4fv(waterShader.location("inverseHulls[0]"),3,GL_FALSE,glm::value_ptr(inverseHulls[0]));
    glUniform1i(waterShader.location("hullMaskEnabled"),1);
    int wakeCount = glm::min((int)wakeTrail.size(),32);
    glUniform1i(waterShader.location("wakeCount"),wakeCount);
    if(wakeCount>0)
        glUniform4fv(waterShader.location("wakeTrail[0]"),wakeCount,glm::value_ptr(wakeTrail[0]));

    if (texMgr && enableTextures)
    {
        texMgr->bind(TEX_FOAM, GL_TEXTURE0);
        glUniform1i(waterShader.location("foamTex"), 0);
        glUniform1i(waterShader.location("useFoamTex"), 1);
    }
    else
    {
        glUniform1i(waterShader.location("useFoamTex"), 0);
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
        glUniform1i(shader.location("useTexture"), 1);
        glUniform1f(shader.location("texScale"), 18.0f);
    }
    // GOURAUD SHADING for smooth sloping coastal bathymetry
    glUniform1i(shader.location("shadingMode"), 1);

    glm::mat4 model = glm::mat4(1.0f);
    GLuint modelLoc = shader.location("model");
    GLuint colorLoc = shader.location("objectColor");
    GLuint specLoc  = shader.location("specularStrength");
    GLuint shinLoc  = shader.location("shininess");
    GLuint unlitLoc = shader.location("unlit");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(colorLoc, 1, glm::value_ptr(sandColor));
    glUniform1f(specLoc, 0.06f);
    glUniform1f(shinLoc, 8.0f);
    glUniform1i(unlitLoc, 0);

    seabedMesh.draw();

    if (texMgr && enableTextures)
    {
        glUniform1i(shader.location("useTexture"), 0);
        texMgr->unbind();
    }

    glUniform1i(shader.location("shadingMode"), 0);
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

    glUniform1i(shader.location("shadingMode"), 0); // Phong
    glm::mat4 model = glm::mat4(1.0f);
    GLuint modelLoc = shader.location("model");
    GLuint colorLoc = shader.location("objectColor");
    GLuint specLoc  = shader.location("specularStrength");
    GLuint shinLoc  = shader.location("shininess");
    GLuint unlitLoc = shader.location("unlit");

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
        glUniform1i(shader.location("useTexture"), 1);
        glUniform1f(shader.location("texScale"), 20.0f);
    }
    // GOURAUD SHADING for smooth color interpolation across sand terrain
    glUniform1i(shader.location("shadingMode"), 1);

    glm::mat4 sandMat = glm::translate(glm::mat4(1.0f), beachCenter);
    sandMat = glm::scale(sandMat, glm::vec3(width, 0.25f, depth));

    // Sand has low specular reflection (matte)
    drawTransformedCube(shader, cube, sandMat, sandColor, 8.0f, 0.08f, 0);

    if (texMgr && enableTextures)
    {
        glUniform1i(shader.location("useTexture"), 0);
        texMgr->unbind();
    }

    // Restore to default Phong
    glUniform1i(shader.location("shadingMode"), 0);
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
// Coastal coconut palm groves along the dunes framing the entire island.
// Outward-leaning trunks curve naturally towards the sea like real atoll palms.
// ============================================================================
inline const std::vector<glm::vec3>& islandBananaPositions() {
    static const std::vector<glm::vec3> positions = {
            glm::vec3(-9.0f, 0.0f, -18.0f), glm::vec3(17.0f, 0.0f, -25.0f),
            glm::vec3(-25.0f, 0.0f, -36.0f), glm::vec3(7.0f, 0.0f, -48.0f),
            glm::vec3(-48.0f, 0.0f, -72.0f), glm::vec3(43.0f, 0.0f, -80.0f),
            glm::vec3(-55.0f, 0.0f, -113.0f), glm::vec3(52.0f, 0.0f, -126.0f),
            glm::vec3(-37.0f, 0.0f, -158.0f), glm::vec3(31.0f, 0.0f, -181.0f),
            glm::vec3(-18.0f, 0.0f, 15.0f), glm::vec3(22.0f, 0.0f, 16.0f),
            glm::vec3(-38.0f, 0.0f, 20.0f), glm::vec3(39.0f, 0.0f, 19.0f),
            glm::vec3(-60.0f, 0.0f, -45.0f), glm::vec3(62.0f, 0.0f, -40.0f),
            glm::vec3(-15.0f, 0.0f, -210.0f), glm::vec3(18.0f, 0.0f, -205.0f)
        };
    return positions;
}

inline const std::vector<glm::vec3>& islandPandanusPositions() {
    static const std::vector<glm::vec3> positions = {
            glm::vec3(28.0f, 0.0f, -20.0f), glm::vec3(-20.0f, 0.0f, -47.0f),
            glm::vec3(19.0f, 0.0f, -61.0f), glm::vec3(-31.0f, 0.0f, -70.0f),
            glm::vec3(61.0f, 0.0f, -54.0f), glm::vec3(-62.0f, 0.0f, -91.0f),
            glm::vec3(60.0f, 0.0f, -111.0f), glm::vec3(-53.0f, 0.0f, -139.0f),
            glm::vec3(44.0f, 0.0f, -166.0f), glm::vec3(-24.0f, 0.0f, -196.0f),
            glm::vec3(-52.0f, 0.0f, 14.0f), glm::vec3(54.0f, 0.0f, 12.0f),
            glm::vec3(-5.0f, 0.0f, 22.0f), glm::vec3(8.0f, 0.0f, 21.0f),
            glm::vec3(-70.0f, 0.0f, -120.0f), glm::vec3(68.0f, 0.0f, -135.0f)
        };
    return positions;
}

inline glm::vec3 safeSolidTreeRoot(float x,float z) {
    // Move only trees encroaching on the seaward driving lane, minimally inland.
    for(int i=0;i<200 && getIslandSignedDistance(x,z)<10.0f;++i){
        glm::vec2 gradient(getIslandSignedDistance(x+.1f,z)-getIslandSignedDistance(x-.1f,z),getIslandSignedDistance(x,z+.1f)-getIslandSignedDistance(x,z-.1f));
        if(glm::length(gradient)<.00001f)break;gradient=glm::normalize(gradient);x+=gradient.x*.1f;z+=gradient.y*.1f;}
    return {x,getCoastalTerrainElevation(x,z),z};
}

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

    // 34 perimeter palms carefully positioned on safe coastal dune ridges (shore ~23-26m)
    // with outward-facing yaw angles so trunks naturally arch out over the beach/water.
    const BackdropTree bgTrees[] = {
        // South-facing dunes (framing the resort and loungers)
        {   0.0f,   14.8f, 1.25f,  3.4f,   90.0f },
        { -15.0f,   18.5f, 1.35f, -3.8f,   75.0f },
        {  16.0f,   18.0f, 1.30f,  3.6f,  105.0f },
        { -28.0f,    8.5f, 1.20f, -3.2f,   65.0f },
        {  29.0f,    9.0f, 1.40f,  4.0f,  115.0f },
        { -42.0f,   -2.0f, 1.30f, -3.5f,   55.0f },
        {  44.0f,   -1.5f, 1.25f,  3.7f,  125.0f },

        // East Coast Dune Ridge (facing the rising sun and eastern reef)
        {  62.0f,  -12.0f, 1.35f,  3.6f,  140.0f },
        {  76.0f,  -28.0f, 1.40f,  3.8f,  150.0f },
        {  86.0f,  -48.0f, 1.25f,  3.2f,  165.0f },
        {  92.0f,  -72.0f, 1.45f,  4.2f,  180.0f },
        {  94.0f, -100.0f, 1.30f,  3.5f,  185.0f },
        {  92.0f, -125.0f, 1.35f,  3.7f,  190.0f },
        {  84.0f, -152.0f, 1.40f,  3.9f,  205.0f },
        {  70.0f, -178.0f, 1.25f,  3.3f,  220.0f },
        {  52.0f, -202.0f, 1.35f,  3.6f,  235.0f },

        // North-East & North Headland (dramatic wind-swept bluffs)
        {  32.0f, -220.0f, 1.40f,  4.0f,  250.0f },
        {  12.0f, -232.0f, 1.30f,  3.5f,  265.0f },
        {  -8.0f, -234.0f, 1.45f, -4.2f,  275.0f },
        { -28.0f, -224.0f, 1.35f, -3.8f,  290.0f },

        // West Coast Dune Ridge (facing the sunset)
        { -48.0f, -204.0f, 1.30f, -3.4f,  305.0f },
        { -66.0f, -180.0f, 1.40f, -3.9f,  320.0f },
        { -78.0f, -154.0f, 1.25f, -3.2f,  335.0f },
        { -88.0f, -128.0f, 1.45f, -4.1f,  350.0f },
        { -90.0f, -100.0f, 1.35f, -3.7f,    0.0f },
        { -86.0f,  -74.0f, 1.30f, -3.5f,   15.0f },
        { -78.0f,  -50.0f, 1.40f, -3.8f,   30.0f },
        { -65.0f,  -26.0f, 1.25f, -3.3f,   45.0f },

        // Elevated Interior Ridge Cluster (multi-tiered horizon layers)
        { -20.0f,  -45.0f, 1.50f,  3.5f,   80.0f },
        {  22.0f,  -46.0f, 1.55f, -3.6f,  100.0f },
        { -35.0f,  -70.0f, 1.60f,  4.0f,   70.0f },
        {  38.0f,  -72.0f, 1.65f, -3.8f,  110.0f },
        { -45.0f, -135.0f, 1.50f,  3.7f,  315.0f },
        {  48.0f, -138.0f, 1.55f, -3.5f,  215.0f }
    };

    for (const auto& t : bgTrees)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), safeSolidTreeRoot(t.x,t.z));
        m = glm::rotate(m, glm::radians(t.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(t.scale));
        drawPalmTree(shader, cube, sphere, m, t.trunkCurve, windSway, texMgr, enableTextures);
    }
}

// ============================================================================
// OBJECT 8D: SAINT MARTIN-INSPIRED ISLAND BACKDROP & DENSE TROPICAL RAINFOREST
// Triangulated island terrain blanketed in a dense, lush tropical rainforest
// with multi-tiered broadleaf canopies and emergent towering coconut palms.
// ============================================================================
inline void drawIslandTerrainBackdrop(
    Shader& shader,
    const IslandTerrainMesh& terrain,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const TextureManager* textures = nullptr,
    bool enableTextures = true,bool includeForest=true)
{
    // Rich Tropical Foliage Color Palette (Layered Rainforest Canopy)
    const glm::vec3 jungleDark(0.040f, 0.20f, 0.07f);    // Deep rainforest understory shadow
    const glm::vec3 jungleMid(0.075f, 0.35f, 0.12f);     // Mature tropical canopy emerald
    const glm::vec3 jungleEmerald(0.11f, 0.45f, 0.14f);  // Lush rainforest mid-canopy
    const glm::vec3 jungleSun(0.18f, 0.54f, 0.16f);      // Sunlit tropical leaves
    const glm::vec3 jungleGolden(0.24f, 0.60f, 0.18f);   // Bright sun-dappled canopy highlights
    const glm::vec3 palmTrunk(0.42f, 0.28f, 0.16f);      // Coconut palm bark
    const glm::vec3 coconutCol(0.28f, 0.16f, 0.08f);     // Tropical coconuts
    const glm::vec3 palmLeaf1(0.14f, 0.58f, 0.16f);      // Vibrant palm frond green
    const glm::vec3 palmLeaf2(0.18f, 0.66f, 0.20f);      // Bright sunlit frond green

    if(!g_forestCapture) {
    glUniform1i(shader.location("terrainPalette"), 1);
    glUniform1i(shader.location("useTexture"), textures && enableTextures);
    if (textures && enableTextures) { textures->bind(TEX_SAND); glUniform1f(shader.location("texScale"), 14.0f); }
    glUniform1i(shader.location("shadingMode"), 1);
    glUniform1f(shader.location("seaLevel"), g_sceneSeaLevel);
    glm::mat4 identity(1.0f);
    glUniformMatrix4fv(shader.location("model"), 1, GL_FALSE, glm::value_ptr(identity));
    glm::vec3 baseColor(0.28f, 0.34f, 0.16f);
    glUniform3fv(shader.location("objectColor"), 1, glm::value_ptr(baseColor));
    glUniform1f(shader.location("specularStrength"), 0.04f);
    glUniform1f(shader.location("shininess"), 7.0f);
    glUniform1i(shader.location("unlit"), 0);
    terrain.draw();
    glUniform1i(shader.location("terrainPalette"), 0);
    glUniform1i(shader.location("useTexture"), 0);
    glUniform1i(shader.location("shadingMode"), 0);
    if (textures && enableTextures) textures->unbind();

    }
    if(!includeForest)return;
    // 1. DENSE MULTI-TIERED TROPICAL CANOPY (480 canopy clusters covering the mountain)
    for (int i = 0; i < 480; ++i)
    {
        float x = -105.0f + 210.0f * islandHash(i + 47, 1);
        float z = -250.0f + 290.0f * islandHash(i + 89, 2);
        float shore = getIslandSignedDistance(x, z);
        if (shore < 25.0f || shore > 92.0f)
            continue;
        // Keep clearing around the elevated lighthouse compound
        float lighthouseDx = x + 6.0f;
        float lighthouseDz = z + 110.0f;
        if (lighthouseDx * lighthouseDx + lighthouseDz * lighthouseDz < 16.0f * 16.0f)
            continue;

        float baseY = getIslandBackdropElevation(x, z);
        if(g_forestCapture)g_forestCapture->roots.emplace_back(x,baseY,z);
        float h = 2.2f + islandHash(i, 3) * 2.8f;
        float crownScale = 1.35f + islandHash(i, 5) * 1.45f; // broad spreading canopy domes

        // Trunk
        glm::mat4 trunk = glm::translate(glm::mat4(1.0f), glm::vec3(x, baseY + h * 0.45f, z));
        trunk = glm::rotate(trunk, glm::radians(islandHash(i, 6) * 12.0f - 6.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        trunk = glm::scale(trunk, glm::vec3(0.28f * crownScale, h, 0.28f * crownScale));
        drawTransformedCube(shader, cube, trunk, glm::vec3(0.22f, 0.15f, 0.08f), 5.0f, 0.02f);

        // 5 Overlapping Canopy Spherical Lobes creating an unbroken green blanket
        const float offsets[5][3] = {
            { -0.78f,  0.05f,  0.20f },
            {  0.72f,  0.10f, -0.15f },
            {  0.05f,  0.75f, -0.22f },
            {  0.15f,  0.22f,  0.68f },
            { -0.20f, -0.15f, -0.55f }
        };
        for (int lobe = 0; lobe < 5; ++lobe)
        {
            float ox = offsets[lobe][0] * crownScale;
            float oy = offsets[lobe][1] * crownScale;
            float oz = offsets[lobe][2] * crownScale;
            glm::mat4 crown = glm::translate(glm::mat4(1.0f), glm::vec3(x + ox, baseY + h + oy, z + oz));
            crown = glm::rotate(crown, glm::radians(islandHash(i + lobe * 17, 7) * 60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            crown = glm::scale(crown, glm::vec3(1.30f, 1.25f, 1.20f) * crownScale);

            glm::vec3 leafCol;
            int colIdx = (i + lobe) % 5;
            if (colIdx == 0) leafCol = jungleDark;
            else if (colIdx == 1) leafCol = jungleMid;
            else if (colIdx == 2) leafCol = jungleEmerald;
            else if (colIdx == 3) leafCol = jungleSun;
            else leafCol = jungleGolden;

            drawTransformedSphere(shader, sphere, crown, leafCol, 6.0f, 0.025f);
        }
    }

    // 2. EMERGENT TOWERING COCONUT PALMS RISING ABOVE CANOPY (Like the reference photo!)
    // Rising 8m to 15m high with graceful leaning curves, visible coconuts and starburst fronds.
    for (int p = 0; p < 130; ++p)
    {
        float px = -100.0f + 200.0f * islandHash(p * 23 + 11, 3);
        float pz = -245.0f + 280.0f * islandHash(p * 29 + 17, 4);
        float pShore = getIslandSignedDistance(px, pz);
        if (pShore < 24.5f || pShore > 88.0f)
            continue;
        float pLhDist = glm::length(glm::vec2(px + 6.0f, pz + 110.0f));
        if (pLhDist < 16.0f)
            continue;

        float groundY = getCoastalTerrainElevation(px, pz);
        if(g_forestCapture)g_forestCapture->roots.emplace_back(px,groundY,pz);
        float palmHeight = 8.5f + islandHash(p, 5) * 6.5f; // 8.5m to 15.0m tall!
        float leanAngle = 8.0f + islandHash(p, 6) * 18.0f; // 8 to 26 deg lean outward
        float outwardYaw = glm::degrees(atan2f(px, pz - (-100.0f))) + (islandHash(p, 7) * 30.0f - 15.0f);

        // Hierarchical Leaning Palm Trunk (6 curved segments)
        glm::mat4 palmNode = glm::translate(glm::mat4(1.0f), glm::vec3(px, groundY, pz));
        palmNode = glm::rotate(palmNode, glm::radians(outwardYaw), glm::vec3(0.0f, 1.0f, 0.0f));

        int segCount = 6;
        float segLen = palmHeight / (float)segCount;
        for (int s = 0; s < segCount; ++s)
        {
            float taper = 1.0f - ((float)s / (float)segCount) * 0.40f;
            glm::mat4 segMat = glm::translate(palmNode, glm::vec3(0.0f, segLen * 0.5f, 0.0f));
            segMat = glm::scale(segMat, glm::vec3(0.26f * taper, segLen * 1.08f, 0.26f * taper));
            drawTransformedSphere(shader, sphere, segMat, (s % 2 == 0) ? palmTrunk : glm::vec3(0.32f, 0.20f, 0.12f), 8.0f, 0.04f);

            palmNode = glm::translate(palmNode, glm::vec3(0.0f, segLen, 0.0f));
            palmNode = glm::rotate(palmNode, glm::radians(leanAngle / (float)segCount), glm::vec3(1.0f, 0.0f, 0.0f));
        }

        // Coconuts nestled at crown
        for (int c = 0; c < 4; ++c)
        {
            float cAngle = (float)c * 90.0f + 20.0f;
            glm::mat4 cMat = glm::rotate(palmNode, glm::radians(cAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            cMat = glm::translate(cMat, glm::vec3(0.18f, -0.06f, 0.0f));
            cMat = glm::scale(cMat, glm::vec3(0.22f, 0.26f, 0.22f));
            drawTransformedSphere(shader, sphere, cMat, coconutCol, 12.0f, 0.08f);
        }

        // Starburst Palm Crown: 11 arching fronds radiating in 360 degrees
        for (int f = 0; f < 11; ++f)
        {
            float frondYaw = (float)f * (360.0f / 11.0f) + islandHash(p, f) * 10.0f;
            glm::mat4 frondBase = glm::rotate(palmNode, glm::radians(frondYaw), glm::vec3(0.0f, 1.0f, 0.0f));
            frondBase = glm::rotate(frondBase, glm::radians(12.0f + islandHash(p, f + 3) * 14.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            float frondLen = 3.6f + islandHash(p, f + 7) * 1.2f;
            glm::mat4 frondStem = glm::translate(frondBase, glm::vec3(0.0f, 0.0f, frondLen * 0.45f));
            frondStem = glm::scale(frondStem, glm::vec3(0.065f, 0.035f, frondLen));
            drawTransformedCube(shader, cube, frondStem, palmTrunk, 16.0f, 0.05f);

            // Drooping feather blades along frond
            for (int seg = 1; seg <= 3; ++seg)
            {
                float zOffset = frondLen * (0.28f * (float)seg);
                float bladeSpan = (1.10f - 0.22f * (float)seg) * 0.75f;
                glm::mat4 blade = glm::translate(frondBase, glm::vec3(0.0f, -0.05f * (float)seg, zOffset));
                blade = glm::rotate(blade, glm::radians(24.0f + 6.0f * (float)seg), glm::vec3(1.0f, 0.0f, 0.0f));
                blade = glm::scale(blade, glm::vec3(bladeSpan * 2.0f, 0.018f, frondLen * 0.28f));
                drawTransformedCube(shader, cube, blade, (f % 2 == 0) ? palmLeaf1 : palmLeaf2, 24.0f, 0.10f);
            }
        }
    }
}
// Conservative clip-space sphere test, shared by color and light-space passes.
inline bool forestInstanceVisible(const ForestInstance& instance,const glm::mat4& clip) {
    glm::vec4 center(instance.model[3]);
    float radius=.5f*(glm::length(glm::vec3(instance.model[0]))+
        glm::length(glm::vec3(instance.model[1]))+glm::length(glm::vec3(instance.model[2])));
    for(int axis=0;axis<3;++axis)for(float sign:{-1.0f,1.0f}) {
        glm::vec4 plane;
        for(int col=0;col<4;++col)plane[col]=clip[col][3]+sign*clip[col][axis];
        if(glm::dot(plane,center)<-radius*glm::length(glm::vec3(plane)))return false;
    }
    return true;
}

class IslandForestBatches {
    struct Batch {
        GLuint vao=0,buffer=0;GLsizei indices=0;
        std::vector<ForestInstance> instances,visible;
        void init(GLuint vertices,GLuint elements,GLsizei count) {
            indices=count;glGenVertexArrays(1,&vao);glGenBuffers(1,&buffer);
            glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vertices);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elements);
            for(int i=0;i<3;++i){glEnableVertexAttribArray(i);glVertexAttribPointer(i,i==2?2:3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(size_t)(i==0?0:i==1?3*sizeof(float):6*sizeof(float)));}
            glBindBuffer(GL_ARRAY_BUFFER,buffer);
            for(int i=0;i<4;++i){glEnableVertexAttribArray(5+i);glVertexAttribPointer(5+i,4,GL_FLOAT,GL_FALSE,sizeof(ForestInstance),(void*)(size_t)(i*sizeof(glm::vec4)));glVertexAttribDivisor(5+i,1);}
            glEnableVertexAttribArray(9);glVertexAttribPointer(9,3,GL_FLOAT,GL_FALSE,sizeof(ForestInstance),(void*)offsetof(ForestInstance,color));glVertexAttribDivisor(9,1);
            glEnableVertexAttribArray(10);glVertexAttribPointer(10,3,GL_FLOAT,GL_FALSE,sizeof(ForestInstance),(void*)offsetof(ForestInstance,material));glVertexAttribDivisor(10,1);glBindVertexArray(0);
        }
        void draw(const glm::mat4& clip) {
            visible.clear();for(const auto& item:instances)if(forestInstanceVisible(item,clip))visible.push_back(item);
            if(visible.empty())return;
            glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,buffer);
            glBufferData(GL_ARRAY_BUFFER,visible.size()*sizeof(ForestInstance),visible.data(),GL_STREAM_DRAW);
            glDrawElementsInstanced(GL_TRIANGLES,indices,GL_UNSIGNED_INT,nullptr,(GLsizei)visible.size());glBindVertexArray(0);
        }
        void cleanup(){glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&buffer);vao=buffer=0;}
    };
    Batch staticCubes,staticSpheres,palmCubes,palmLeaves,palmSpheres;
    float lastWind=1000;
    std::vector<glm::vec3> staticRoots,palmRoots;
public:
    void init(Shader& shader,const IslandTerrainMesh& terrain,const CubeMesh& cube,const SphereMesh& sphere) {
        staticCubes.init(cube.VBO,cube.EBO,cube.indexCount);staticSpheres.init(sphere.VBO,sphere.EBO,sphere.indexCount);
        palmCubes.init(cube.VBO,cube.EBO,cube.indexCount);palmLeaves.init(cube.VBO,cube.EBO,cube.indexCount);palmSpheres.init(sphere.VBO,sphere.EBO,sphere.indexCount);
        ForestCapture capture;g_forestCapture=&capture;
        drawIslandTerrainBackdrop(shader,terrain,cube,sphere);g_forestCapture=nullptr;
        staticCubes.instances=std::move(capture.cubes);staticSpheres.instances=std::move(capture.spheres);
        staticRoots=std::move(capture.roots);
    }
    void update(Shader& shader,const CubeMesh& cube,const SphereMesh& sphere,float wind) {
        if(wind==lastWind)return;lastWind=wind;
        ForestCapture capture;g_forestCapture=&capture;
        drawBackgroundPalmForest(shader,cube,sphere,wind);g_forestCapture=nullptr;
        palmCubes.instances=std::move(capture.cubes);palmLeaves.instances=std::move(capture.leaves);palmSpheres.instances=std::move(capture.spheres);
        palmRoots=std::move(capture.roots);
    }
    size_t instanceCount()const{return staticCubes.instances.size()+staticSpheres.instances.size()+palmCubes.instances.size()+palmLeaves.instances.size()+palmSpheres.instances.size();}
    std::vector<glm::vec3> roots()const {auto result=staticRoots;result.insert(result.end(),palmRoots.begin(),palmRoots.end());return result;}
    void draw(Shader& shader,const TextureManager& textures,bool textured) {
        glm::mat4 view,proj;glGetUniformfv(shader.ID,shader.location("view"),glm::value_ptr(view));glGetUniformfv(shader.ID,shader.location("proj"),glm::value_ptr(proj));
        glUniform1i(shader.location("instancedPass"),1);glUniform1i(shader.location("shadingMode"),0);glUniform1i(shader.location("useTexture"),0);
        auto clip=proj*view;staticCubes.draw(clip);staticSpheres.draw(clip);palmCubes.draw(clip);palmSpheres.draw(clip);
        if(textured){textures.bind(TEX_PALM);glUniform1i(shader.location("useTexture"),1);glUniform1f(shader.location("texScale"),1);}
        palmLeaves.draw(clip);
        if(textured)textures.unbind();glUniform1i(shader.location("useTexture"),0);glUniform1i(shader.location("instancedPass"),0);
    }
    void cleanup(){staticCubes.cleanup();staticSpheres.cleanup();palmCubes.cleanup();palmLeaves.cleanup();palmSpheres.cleanup();}
};

// OBJECT 8E: ROBOT-RIDDEN BEACH QUAD
// Four knobby driven tyres, front steering, suspension, tubular racks,
// four auxiliary lamps, handlebars, mirrors and a seated robot rider.
// ============================================================================
// Local +Z is the front, matching heading and camera navigation.
inline void drawBeach4WD(
    Shader& shader,const CubeMesh& cube,const SphereMesh& sphere,
    const FrustumMesh& wheelMesh,const glm::mat4& groundTransform,
    float wheelSpin=0,float steering=0,float riderLook=0,bool includeRider=true,
    float bodyPitch=0,float bodyRoll=0)
{
    glm::mat4 baseTransform=glm::translate(groundTransform,glm::vec3(0,.5f,0));
    baseTransform=glm::rotate(baseTransform,glm::radians(bodyPitch),glm::vec3(1,0,0));
    baseTransform=glm::rotate(baseTransform,glm::radians(bodyRoll),glm::vec3(0,0,1));
    baseTransform=glm::translate(baseTransform,glm::vec3(0,-.5f,0));
    const glm::vec3 green(0.16f,0.28f,0.15f),black(0.045f,0.052f,0.050f);
    const glm::vec3 rubber(0.025f,0.029f,0.027f),chrome(0.62f,0.67f,0.69f);
    auto box=[&](glm::vec3 p,glm::vec3 size,glm::vec3 color,float gloss=24.0f) {
        glm::mat4 m=glm::scale(glm::translate(baseTransform,p),size);
        drawTransformedCube(shader,cube,m,color,gloss,color==chrome?0.65f:0.18f);
    };
    auto shell=[&](glm::vec3 p,glm::vec3 size,glm::vec3 color,float gloss=48.0f) {
        drawTransformedSphere(shader,sphere,glm::scale(glm::translate(baseTransform,p),size),color,gloss,.25f);
    };
    auto panel=[&](glm::vec3 p,glm::vec3 size,glm::vec3 color) {
        atvPanel().draw(shader,glm::scale(glm::translate(baseTransform,p),size),color);
    };
    auto rod=[&](glm::vec3 from,glm::vec3 to,float diameter,glm::vec3 color) {
        glm::vec3 direction=to-from;float length=glm::length(direction);
        glm::vec3 axis=glm::cross(glm::vec3(0,1,0),direction/length);
        glm::mat4 m=glm::translate(baseTransform,(from+to)*0.5f);
        if(glm::length(axis)>0.001f)
            m=glm::rotate(m,acosf(glm::clamp(direction.y/length,-1.0f,1.0f)),glm::normalize(axis));
        else if(direction.y<0)m=glm::rotate(m,3.14159265f,glm::vec3(1,0,0));
        drawTransformedFrustum(shader,wheelMesh,glm::scale(m,glm::vec3(diameter,length,diameter)),color,52,0.4f);
    };
    // Low engine, broad footboards, saddle and sculpted mudguards.
    box(glm::vec3(0,.53f,0),glm::vec3(.80f,.25f,1.72f),black);
    box(glm::vec3(0,.76f,-.12f),glm::vec3(.48f,.45f,.72f),black);
    panel(glm::vec3(0,.93f,.65f),glm::vec3(.94f,.34f,.78f),green);
    panel(glm::vec3(0,.90f,-.76f),glm::vec3(.95f,.26f,.65f),green);
    shell(glm::vec3(0,1.04f,-.12f),glm::vec3(.47f,.23f,1.04f),rubber,16);
    panel(glm::vec3(0,.99f,.18f),glm::vec3(.50f,.37f,.63f),green);
    for(int side:{-1,1})
    {
        box(glm::vec3(side*.53f,.53f,-.12f),glm::vec3(.31f,.10f,.87f),black);
        for(float z:{-.78f,.78f})
        {
            glm::mat4 fender=glm::translate(baseTransform,glm::vec3(side*.64f,.93f,z));
            fender=glm::rotate(fender,glm::radians(z>0?-9.0f:9.0f),glm::vec3(1,0,0));
            atvPanel().draw(shader,glm::scale(fender,glm::vec3(.46f,.20f,.82f)),green);
            glm::mat4 mudflap=glm::translate(baseTransform,glm::vec3(side*.64f,.72f,z-.32f));
            drawTransformedCube(shader,cube,glm::scale(mudflap,glm::vec3(.36f,.31f,.035f)),black,12,.08f);
            // Visible double wishbones and shock absorber, attached to each wheel.
            rod(glm::vec3(side*.22f,.56f,z),glm::vec3(side*.64f,.43f,z),.045f,black);
            rod(glm::vec3(side*.22f,.69f,z-.12f),glm::vec3(side*.64f,.50f,z),.035f,chrome);
            glm::vec3 lower(side*.51f,.46f,z),upper(side*.35f,.87f,z-.12f);
            rod(lower,upper,.065f,chrome);
            // Short helical spring around the shock axis.
            glm::vec3 axis=glm::normalize(upper-lower),u=glm::normalize(glm::cross(axis,glm::vec3(0,0,1))),v=glm::cross(axis,u);
            glm::vec3 previous=lower+u*.065f;
            for(int k=1;k<=24;++k)
            {
                float t=(float)k/24,a=t*6.2831853f*4;
                glm::vec3 point=glm::mix(lower,upper,t)+(u*cosf(a)+v*sinf(a))*.065f;
                rod(previous,point,.014f,chrome);previous=point;
            }
        }
    }
    // Four driven knobby tyres. Front tyres steer; all rotate with ground speed.
    for(float z:{-.78f,.78f}) for(int side:{-1,1})
    {
        glm::mat4 pivot=glm::translate(groundTransform,glm::vec3(side*.72f,.45f,z));
        if(z>0)
        {
            float angle=glm::radians(steering);
            // Inner front tyre turns farther than the outer one (Ackermann).
            if(fabsf(angle)>.001f)
            {
                float radius=1.56f/tanf(angle);
                angle=atanf(1.56f/(radius-side*.72f));
            }
            pivot=glm::rotate(pivot,angle,glm::vec3(0,1,0));
        }
        pivot=glm::rotate(pivot,glm::radians(wheelSpin),glm::vec3(1,0,0));
        glm::mat4 cylinder=glm::rotate(pivot,glm::radians(90.0f),glm::vec3(0,0,1));
        drawTransformedFrustum(shader,wheelMesh,glm::scale(cylinder,glm::vec3(.86f,.36f,.86f)),rubber,12,.07f);
        for(int lug=0;lug<16;++lug) for(int row:{-1,1})
        {
            float angle=lug*6.2831853f/16+(row>0?.09f:0);
            glm::mat4 tread=glm::translate(pivot,glm::vec3(row*.105f,sinf(angle)*.424f,cosf(angle)*.424f));
            tread=glm::rotate(tread,-angle,glm::vec3(1,0,0));
            tread=glm::rotate(tread,glm::radians(row*18.0f),glm::vec3(0,1,0));
            drawTransformedCube(shader,cube,glm::scale(tread,glm::vec3(.15f,.095f,.09f)),rubber,10,.04f);
        }
        glm::mat4 hub=glm::translate(pivot,glm::vec3(side*.187f,0,0));
        hub=glm::rotate(hub,glm::radians(90.0f),glm::vec3(0,0,1));
        drawTransformedFrustum(shader,wheelMesh,glm::scale(hub,glm::vec3(.49f,.035f,.49f)),chrome,90,.7f);
        glm::mat4 recess=glm::translate(pivot,glm::vec3(side*.210f,0,0));
        recess=glm::rotate(recess,glm::radians(90.0f),glm::vec3(0,0,1));
        drawTransformedFrustum(shader,wheelMesh,glm::scale(recess,glm::vec3(.37f,.013f,.37f)),black,30,.15f);
        for(int spoke=0;spoke<6;++spoke)
        {
            glm::mat4 m=glm::translate(pivot,glm::vec3(side*.225f,0,0));
            m=glm::rotate(m,glm::radians(spoke*60.0f),glm::vec3(1,0,0));
            m=glm::translate(m,glm::vec3(0,.13f,0));
            drawTransformedCube(shader,cube,glm::scale(m,glm::vec3(.024f,.24f,.036f)),chrome,90,.7f);
        }
    }
    // Open tubular cargo racks, brush guard, and front radiator grille.
    for(float z:{-.90f,.79f})
    {
        for(float x:{-.49f,.49f})rod(glm::vec3(x,1.14f,z-.20f),glm::vec3(x,1.14f,z+.20f),.038f,black);
        for(float dz:{-.20f,0.0f,.20f})rod(glm::vec3(-.49f,1.14f,z+dz),glm::vec3(.49f,1.14f,z+dz),.038f,black);
    }
    box(glm::vec3(0,.82f,1.02f),glm::vec3(.60f,.26f,.06f),black);
    for(int i=0;i<5;++i)box(glm::vec3(-.22f+i*.11f,.82f,1.056f),glm::vec3(.025f,.22f,.016f),chrome);
    rod(glm::vec3(-.50f,.51f,1.15f),glm::vec3(.50f,.51f,1.15f),.065f,black);
    for(int side:{-1,1})
    {
        rod(glm::vec3(side*.48f,.51f,1.15f),glm::vec3(side*.42f,.91f,1.13f),.065f,black);
        box(glm::vec3(side*.38f,.95f,1.036f),glm::vec3(.18f,.15f,.07f),glm::vec3(.8f,.9f,.88f),80);
        box(glm::vec3(side*.47f,1.04f,1.028f),glm::vec3(.08f,.055f,.045f),glm::vec3(.85f,.32f,.04f));
        box(glm::vec3(side*.35f,.94f,-1.065f),glm::vec3(.18f,.085f,.045f),glm::vec3(.62f,.035f,.02f));
    }
    // Four round auxiliary lamps across the front rack, as in the reference.
    for(int i=0;i<4;++i)
    {
        float x=-.33f+i*.22f;
        rod(glm::vec3(x,1.17f,1.0f),glm::vec3(x,1.32f,1.0f),.03f,black);
        glm::mat4 lamp=glm::translate(baseTransform,glm::vec3(x,1.34f,1.0f));
        lamp=glm::rotate(lamp,glm::radians(90.0f),glm::vec3(1,0,0));
        drawTransformedFrustum(shader,wheelMesh,glm::scale(lamp,glm::vec3(.19f,.10f,.19f)),black,40,.3f);
        glm::mat4 lens=glm::translate(baseTransform,glm::vec3(x,1.34f,1.058f));
        lens=glm::rotate(lens,glm::radians(90.0f),glm::vec3(1,0,0));
        drawTransformedFrustum(shader,wheelMesh,glm::scale(lens,glm::vec3(.15f,.013f,.15f)),glm::vec3(.77f,.84f,.78f),100,.8f);
    }
    // Handlebars turn with the front axle; mirrors sit above the rider's hands.
    glm::mat4 controls=glm::translate(baseTransform,glm::vec3(0,1.42f,.22f));
    controls=glm::rotate(controls,glm::radians(steering*.65f),glm::vec3(0,1,0));
    for(int side:{-1,1})
    {
        glm::mat4 grip=glm::translate(controls,glm::vec3(side*.33f,0,.025f));
        drawTransformedCube(shader,cube,glm::scale(grip,glm::vec3(.22f,.055f,.055f)),rubber,12,.08f);
        rod(glm::vec3(side*.23f,1.40f,.24f),glm::vec3(side*.36f,1.66f,.46f),.025f,black);
        glm::mat4 mirror=glm::translate(baseTransform,glm::vec3(side*.38f,1.70f,.47f));
        drawTransformedSphere(shader,sphere,glm::scale(mirror,glm::vec3(.15f,.19f,.05f)),black,75,.4f);
    }
    rod(glm::vec3(-.38f,1.41f,.24f),glm::vec3(.38f,1.41f,.24f),.042f,chrome);
    rod(glm::vec3(0,1.41f,.24f),glm::vec3(0,.87f,.43f),.065f,black);
    if(includeRider)
    {
        glm::mat4 rider=glm::translate(baseTransform,glm::vec3(0,1.20f,-.15f));
        rider=glm::scale(rider,glm::vec3(.85f));
        drawRobot(shader,cube,sphere,rider,glm::clamp(riderLook+steering*.45f,-75.0f,75.0f),
                  -4.0f,-58.0f,-58.0f,-65.0f,.35f);
    }
}

// ============================================================================
// ROADSIDE SOLAR ATV CHARGING BAY
// A human-scale charger with display, tethered cable, protected parking bay
// and photovoltaic canopy, placed off the coastal driving lane.
// ============================================================================
inline void drawATVChargingStation(Shader& shader,const CubeMesh& cube,
    const FrustumMesh& cylinder,const glm::mat4& base,bool connected=false,float batteryPercent=60,
    glm::vec3 vehiclePort=glm::vec3(0))
{
    const glm::vec3 steel(.16f,.20f,.21f),white(.77f,.81f,.79f),yellow(.89f,.64f,.12f);
    auto box=[&](glm::vec3 p,glm::vec3 size,glm::vec3 color,int glow=0) {
        drawTransformedCube(shader,cube,glm::scale(glm::translate(base,p),size),color,36,.22f,glow);
    };
    auto tube=[&](glm::vec3 a,glm::vec3 b,float diameter,glm::vec3 color) {
        glm::vec3 d=b-a;float length=glm::length(d);
        glm::mat4 m=glm::translate(base,(a+b)*.5f);
        glm::vec3 axis=glm::cross(glm::vec3(0,1,0),d/length);
        if(glm::length(axis)>.001f)m=glm::rotate(m,acosf(glm::clamp(d.y/length,-1.0f,1.0f)),glm::normalize(axis));
        else if(d.y<0)m=glm::rotate(m,3.14159265f,glm::vec3(1,0,0));
        drawTransformedFrustum(shader,cylinder,glm::scale(m,glm::vec3(diameter,length,diameter)),color,24,.15f);
    };
    // 3.6 x 4.6 m lay-by; nearest edge remains clear of the driving lane.
    box(glm::vec3(0,-.12f,0),glm::vec3(3.6f,.28f,4.6f),glm::vec3(.48f,.48f,.43f));
    for(float x:{-1.55f,.65f})box(glm::vec3(x,.027f,0),glm::vec3(.06f,.016f,3.5f),yellow);
    box(glm::vec3(-.45f,.027f,-1.7f),glm::vec3(2.2f,.016f,.06f),yellow);
    // Charger stands inland of the empty parking space, not in front of it.
    box(glm::vec3(1.23f,.83f,.65f),glm::vec3(.53f,1.65f,.46f),white);
    box(glm::vec3(1.23f,.20f,.65f),glm::vec3(.60f,.40f,.52f),steel);
    box(glm::vec3(.954f,1.22f,.65f),glm::vec3(.025f,.39f,.32f),steel);
    box(glm::vec3(.937f,1.24f,.65f),glm::vec3(.012f,.25f,.24f),connected?glm::vec3(.04f,.5f,.16f):glm::vec3(.04f,.33f,.35f),1);
    for(int i=0;i<4;++i)box(glm::vec3(.928f,1.20f,.56f+i*.058f),glm::vec3(.014f,.09f,.034f),
        connected && batteryPercent<(i+1)*25?glm::vec3(.06f,.13f,.09f):glm::vec3(.18f,.85f,.54f),1);
    box(glm::vec3(.95f,.95f,.65f),glm::vec3(.025f,.035f,.20f),glm::vec3(.15f,.9f,.55f),1);
    // Hung charging cable, ending in a holstered connector beside the display.
    glm::vec3 previous(.96f,.82f,.79f);
    glm::vec3 socket(glm::inverse(base)*glm::vec4(vehiclePort,1));
    for(int i=1;i<=20;++i)
    {
        float t=i/20.0f;
        glm::vec3 next=connected?glm::mix(glm::vec3(.96f,.82f,.79f),socket,t)-glm::vec3(0,.22f*sinf(t*3.14159265f),0):
            glm::vec3(.91f-.12f*sinf(t*3.14159265f),.82f-.60f*sinf(t*3.14159265f),.79f-.27f*t);
        tube(previous,next,.035f,glm::vec3(.035f));previous=next;
    }
    box(connected?socket:glm::vec3(.91f,.85f,.52f),glm::vec3(.075f,.17f,.075f),steel);
    for(float z:{-.15f,1.48f})tube(glm::vec3(1.24f,.03f,z),glm::vec3(1.24f,.67f,z),.10f,yellow);
    // Canopy posts are only on the inland side, leaving the entry open.
    for(float z:{-1.9f,1.9f})
    {
        tube(glm::vec3(1.58f,.02f,z),glm::vec3(1.58f,2.85f,z),.11f,steel);
        tube(glm::vec3(1.58f,2.1f,z),glm::vec3(.75f,2.85f,z),.065f,steel);
    }
    box(glm::vec3(0,2.9f,0),glm::vec3(3.9f,.12f,4.8f),steel);
    for(int x=0;x<4;++x)for(int z=0;z<6;++z)
        box(glm::vec3(-1.44f+x*.96f,2.976f,-1.98f+z*.79f),glm::vec3(.90f,.028f,.73f),glm::vec3(.035f,.09f,.16f));
    // Green roadside sign and a recognisable lightning bolt (no bitmap text).
    box(glm::vec3(-1.96f,2.65f,0),glm::vec3(.10f,.42f,.64f),glm::vec3(.06f,.31f,.19f));
    for(int i=0;i<3;++i)
    {
        glm::mat4 bolt=glm::translate(base,glm::vec3(-2.02f,2.77f-i*.11f,i==1?.035f:-.025f));
        bolt=glm::rotate(bolt,glm::radians(i==1?-65.0f:25.0f),glm::vec3(1,0,0));
        drawTransformedCube(shader,cube,glm::scale(bolt,glm::vec3(.016f,.16f,.045f)),glm::vec3(.65f,1,.78f),16,.1f,1);
    }
}

// ============================================================================
// OBJECT 8F: COASTAL LIGHTHOUSE
// Banded masonry tower, glazed lantern room, gallery, roof and glowing optic.
// ============================================================================
struct LighthouseLanternDimensions {
    static constexpr float radius=1.525f,bottom=14.275f,top=16.575f,opticY=15.55f;
    static constexpr float beamTipRadius=.0125f;
};
inline glm::mat4 lighthouseWindowTransform(const glm::mat4& base,int pane,float y)
{
    float angle=(pane+.5f)*6.283185307f/12,apothem=LighthouseLanternDimensions::radius*cosf(3.141592654f/12);
    glm::mat4 m=glm::translate(base,{apothem*cosf(angle),y,apothem*sinf(angle)});
    return glm::rotate(m,1.570796327f-angle,glm::vec3(0,1,0));
}

// Transparent effects must not leak depth/blend/cull or material flags into
// the ocean, the next scene object or the interface.
struct LighthouseTransparentState {
    Shader& shader;GLboolean depth,blend,cull,mask;
    GLint src,dst,srcAlpha,dstAlpha,oldPass=0,oldUnlit=0,oldTexture=0;float alpha=1;
    explicit LighthouseTransparentState(Shader& s):shader(s){
        depth=glIsEnabled(GL_DEPTH_TEST);blend=glIsEnabled(GL_BLEND);cull=glIsEnabled(GL_CULL_FACE);
        glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);glGetIntegerv(GL_BLEND_SRC_RGB,&src);glGetIntegerv(GL_BLEND_DST_RGB,&dst);
        glGetIntegerv(GL_BLEND_SRC_ALPHA,&srcAlpha);glGetIntegerv(GL_BLEND_DST_ALPHA,&dstAlpha);
        glGetUniformfv(s.ID,s.location("objectAlpha"),&alpha);
        glGetUniformiv(s.ID,s.location("beaconBeamPass"),&oldPass);glGetUniformiv(s.ID,s.location("unlit"),&oldUnlit);
        glGetUniformiv(s.ID,s.location("useTexture"),&oldTexture);
        glEnable(GL_DEPTH_TEST);glEnable(GL_BLEND);glDisable(GL_CULL_FACE);glDepthMask(GL_FALSE);
        glUniform1i(s.location("useTexture"),0);
    }
    ~LighthouseTransparentState(){
        glDepthMask(mask);if(!depth)glDisable(GL_DEPTH_TEST);if(!blend)glDisable(GL_BLEND);if(cull)glEnable(GL_CULL_FACE);
        glBlendFuncSeparate(src,dst,srcAlpha,dstAlpha);
        glUniform1f(shader.location("objectAlpha"),alpha);glUniform1i(shader.location("beaconBeamPass"),oldPass);
        glUniform1i(shader.location("unlit"),oldUnlit);glUniform1i(shader.location("useTexture"),oldTexture);
    }
};
inline void drawLighthouse(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const FrustumMesh& towerMesh,
    const FrustumMesh& cylinderMesh,
    const FrustumMesh& roofMesh,
    const glm::mat4& baseTransform,
    float time,
    float nightIntensity=0)
{
    const glm::vec3 whiteStone(0.88f, 0.86f, 0.78f);
    const glm::vec3 redBand(0.72f, 0.07f, 0.055f);
    const glm::vec3 darkMetal(0.10f, 0.12f, 0.12f);
    const glm::vec3 lanternGlass(0.20f, 0.50f, 0.58f);

    // 14 m tapered masonry shaft: 4.6 m diameter at the base, 3.1 m at top.
    glm::mat4 tower = glm::translate(baseTransform, glm::vec3(0.0f, 7.0f, 0.0f));
    tower = glm::scale(tower, glm::vec3(4.6f, 14.0f, 4.6f));
    // Paint on the shaft itself, rather than intersecting straight cylinders
    // with a tapered wall (which produced jagged, flickering stripe edges).
    glUniform1i(shader.location("lighthouseShaftPass"), 1);
    drawTransformedFrustum(shader, towerMesh, tower, whiteStone, 24.0f, 0.18f);
    glUniform1i(shader.location("lighthouseShaftPass"), 0);

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

    const float bottom=LighthouseLanternDimensions::bottom,top=LighthouseLanternDimensions::top;
    const float radius=LighthouseLanternDimensions::radius;
    float chord=2*radius*sinf(3.141592654f/12);
    // Twelve window bays: no solid cylinder concealing the optic.
    for(int i=0;i<12;++i){
        float a=i*6.283185307f/12;
        auto upright=glm::translate(baseTransform,glm::vec3(radius*cosf(a),(bottom+top)*.5f,radius*sinf(a)));
        drawTransformedCube(shader,cube,glm::scale(upright,{.075f,top-bottom,.075f}),darkMetal,38,.35f);
        for(float y:{bottom+.045f,top-.045f})
            drawTransformedCube(shader,cube,glm::scale(lighthouseWindowTransform(baseTransform,i,y),
                {chord+.035f,.09f,.075f}),darkMetal,38,.35f);
    }

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
    for(int i=0;i<12;++i){
        float a=(i+.5f)*6.283185307f/12,r=2.45f*cosf(3.141592654f/12);
        auto rail=glm::translate(baseTransform,glm::vec3(r*cosf(a),15.23f,r*sinf(a)));
        rail=glm::rotate(rail,1.570796327f-a,glm::vec3(0,1,0));
        drawTransformedCube(shader,cube,glm::scale(rail,{2*2.45f*sinf(3.141592654f/12)+.04f,.10f,.085f}),darkMetal,28,.28f);
    }

    glm::mat4 roof = glm::translate(baseTransform, glm::vec3(0.0f, 17.30f, 0.0f));
    roof = glm::scale(roof, glm::vec3(3.65f, 1.45f, 3.65f));
    drawTransformedFrustum(shader, roofMesh, roof, redBand, 38.0f, 0.35f);

    // The lamp itself is steady; the rotating focused optic creates the flash.
    auto pedestal=glm::translate(baseTransform,glm::vec3(0,14.89f,0));
    drawTransformedCube(shader,cube,glm::scale(pedestal,{.28f,1.05f,.28f}),darkMetal,28,.2f);
    glm::mat4 optic=glm::translate(baseTransform,glm::vec3(0,LighthouseLanternDimensions::opticY,0));
    optic=glm::scale(optic,glm::vec3(.80f));
    glm::vec3 lamp=glm::mix(glm::vec3(.72f,.62f,.38f),glm::vec3(1,.91f,.58f),glm::clamp(nightIntensity,0.f,1.f));
    drawTransformedSphere(shader,sphere,optic,lamp,1,0,1);
}

inline void drawLighthouseGlass(Shader& shader,const CubeMesh& cube,const glm::mat4& base,
    glm::vec3 cameraPosition,float nightIntensity=0)
{
    LighthouseTransparentState saved(shader);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(shader.location("beaconBeamPass"),0);
    std::vector<std::pair<float,int>> sorted;
    const float y=(LighthouseLanternDimensions::bottom+LighthouseLanternDimensions::top)*.5f;
    for(int i=0;i<12;++i){auto pane=lighthouseWindowTransform(base,i,y);
        auto d=glm::vec3(pane[3])-cameraPosition;sorted.emplace_back(glm::dot(d,d),i);}
    std::sort(sorted.begin(),sorted.end(),[](const auto& a,const auto& b){return a.first>b.first;});
    float width=2*LighthouseLanternDimensions::radius*sinf(3.141592654f/12)-.075f;
    glUniform1f(shader.location("objectAlpha"),.075f);
    for(const auto& entry:sorted){
        auto pane=glm::scale(lighthouseWindowTransform(base,entry.second,y),
            glm::vec3(width,LighthouseLanternDimensions::top-LighthouseLanternDimensions::bottom-.18f,.018f));
        drawTransformedCube(shader,cube,pane,glm::mix(glm::vec3(.44f,.67f,.74f),glm::vec3(.70f,.76f,.78f),nightIntensity),96,.65f);
    }
}

// Rotating lighthouse beacon. The cone is additive and does not write depth,
// so it reads as illuminated mist while still being occluded by solid terrain.
inline void drawLighthouseBeacon(
    Shader& shader,
    const FrustumMesh& coneMesh,
    const glm::mat4& lighthouseTransform,
    float time,
    float nightIntensity,
    float headingDegrees = -1.0f,
    float elevationDegrees = 0.0f)
{
    if (nightIntensity <= 0.01f)
        return;

    const float beamLength = 300.0f;
    // One revolution in roughly 6.5 seconds, producing a recognizable flash
    // characteristic whenever the narrow beam passes an observer.
    float sweep = headingDegrees>=0?headingDegrees:time * 55.0f;
    glm::mat4 beam = glm::translate(lighthouseTransform, glm::vec3(0.0f, LighthouseLanternDimensions::opticY, 0.0f));
    beam = glm::rotate(beam, glm::radians(sweep), glm::vec3(0.0f, 1.0f, 0.0f));
    // Tilt around the optic, before translating to the cone centre, so aiming
    // never moves the beam's origin away from the lantern.
    beam = glm::rotate(beam, glm::radians(elevationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    beam = glm::translate(beam, glm::vec3(beamLength * 0.5f, 0.0f, 0.0f));
    beam = glm::rotate(beam, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    LighthouseTransparentState saved(shader);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glUniform1i(shader.location("beaconBeamPass"), 1);

    // The optic has small vertical divergence but a somewhat wider horizontal
    // fan. This avoids the previous pair of thick, parallel luminous rails.
    glUniform1f(shader.location("objectAlpha"), 0.045f * nightIntensity);
    glm::mat4 halo = glm::scale(beam, glm::vec3(12.0f, beamLength, 34.0f));
    drawTransformedFrustum(shader, coneMesh, halo, glm::vec3(0.72f, 0.84f, 1.0f), 1.0f, 0.0f, 1);

    glUniform1f(shader.location("objectAlpha"), 0.16f * nightIntensity);
    glm::mat4 core = glm::scale(beam, glm::vec3(6.0f, beamLength, 18.0f));
    drawTransformedFrustum(shader, coneMesh, core, glm::vec3(1.0f, 0.96f, 0.74f), 1.0f, 0.0f, 1);

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
    glUniform1i(shader.location("shadingMode"), 2);

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
    glUniform1i(shader.location("shadingMode"), 0);
}

// Organic standalone rock cluster positioned relative to baseTransform (ideal for showcase)
inline void drawRockCluster(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform)
{
    // FLAT SHADING for faceted rock cluster
    glUniform1i(shader.location("shadingMode"), 2);

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
    glUniform1i(shader.location("shadingMode"), 0);
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

// ============================================================================
// OBJECT 11: 3D RAISED TROPICAL STILT HOUSE (ACCURATE ARCHITECTURAL GEOMETRY)
// Modeled faithfully after traditional Southeast Asian / Pacific raised stilt architecture
// (matching the user reference photo):
// 1. Foundation & Stilts: 10 heavy cylindrical wooden columns on square concrete
//    footings adaptive to terrain elevation, with clean diagonal cross-bracing.
// 2. Main Raised Deck: Longitudinal girder beams, 9 transverse floor joists,
//    perimeter edge fascia, and 24 individual wooden deck floor planks.
// 3. Walls & Rooms: Woven bamboo wall screens, rear and side window openings with
//    awning shutters propped open with wooden sticks, open center veranda doorway.
// 4. Open-Air Front Veranda: 4 heavy structural veranda posts, top lintel beam,
//    perimeter balustrades (top/bottom rails + vertical balusters), deck armchair,
//    side table, terracotta water jug, and draped sarong towel.
// 5. Signature Traditional Pitched Thatch Roof:
//    - Both front and back slopes slope DOWNWARDS from the ridge beam to eaves.
//    - Deep 0.90m overhangs protecting walls and veranda from tropical weather.
//    - Seamless triangular gable walls using PrismMesh closing the left and right ends.
//    - Heavy cylindrical ridge log and woven thatch ridge crest.
//    - 18 exposed under-eave structural rafters and tiered thatch eaves fringes.
// 6. Staircases:
//    - Left Main Entrance Staircase: 10 solid wooden step treads with risers, dual
//      heavy angled stringers (+36.9 deg incline), handrail posts, and smooth continuous
//      top rails cleanly flush-landing on the veranda deck edge.
//    - Right Secondary Staircase: 5 steps with side stringer and handrail.
// 7. Lush Tropical Environment:
//    - Raised timber planter bed with rich garden soil and 5 multi-frond tropical
//      broadleaf plant clusters and vibrant flowering shoots.
//    - Towering organic shade tree behind/beside the left staircase, leaning over the roof.
//    - Peaceful sleeping dog curled up on a woven straw mat near the stairs base.
// ============================================================================
// Returns local-space bottom/top for a footing enclosing the sampled terrain.
// Its underside penetrates the lowest sample; its top clears the highest one.
inline glm::vec2 terrainSupportRange(const glm::mat4& base,float x,float z,float halfX,float halfZ,float thickness=.20f) {
    float low=1e30f,high=-1e30f;
    for(int i=0;i<=4;++i)for(int j=0;j<=4;++j){
        glm::vec4 p=base*glm::vec4(x-halfX+2*halfX*i/4,0,z-halfZ+2*halfZ*j/4,1);
        float y=getCoastalTerrainElevation(p.x,p.z)-base[3].y;low=glm::min(low,y);high=glm::max(high,y);}
    return {low-.04f,high+thickness-.04f};
}

inline void drawTropicalStiltHouse(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const FrustumMesh& cylinder,
    const FrustumMesh& cone,
    const PrismMesh& prism,
    const glm::mat4& baseTransform)
{
    // Authentic Material Color Palette
    const glm::vec3 woodDark     = glm::vec3(0.32f, 0.22f, 0.13f); // Weathered structural timber
    const glm::vec3 woodMedium   = glm::vec3(0.42f, 0.29f, 0.17f); // Columns, beams, floor planks
    const glm::vec3 woodLight    = glm::vec3(0.52f, 0.38f, 0.24f); // Rails, stairs, furniture
    const glm::vec3 bambooWeave  = glm::vec3(0.76f, 0.62f, 0.38f); // Woven bamboo wall screens
    const glm::vec3 bambooTrim   = glm::vec3(0.64f, 0.50f, 0.28f); // Window frames and wall battens
    const glm::vec3 thatchRoof   = glm::vec3(0.52f, 0.42f, 0.26f); // Main dried thatch roof slopes
    const glm::vec3 thatchFringe = glm::vec3(0.42f, 0.32f, 0.19f); // Thatch ridge cap & eave fringes
    const glm::vec3 concreteCol  = glm::vec3(0.66f, 0.64f, 0.60f); // Foundation footings
    const glm::vec3 soilCol      = glm::vec3(0.22f, 0.16f, 0.10f); // Rich dark garden soil
    const glm::vec3 leafDark     = glm::vec3(0.14f, 0.38f, 0.12f); // Deep jungle green leaves
    const glm::vec3 leafBright   = glm::vec3(0.24f, 0.54f, 0.16f); // Vibrant emerald leaf fronds
    const glm::vec3 flowerRed    = glm::vec3(0.90f, 0.32f, 0.14f); // Tropical heliconia flower bracts
    const glm::vec3 flowerGold   = glm::vec3(0.96f, 0.76f, 0.18f); // Golden flower tips
    const glm::vec3 treeBark     = glm::vec3(0.35f, 0.24f, 0.15f); // Shade tree weathered trunk
    const glm::vec3 treeFoliage  = glm::vec3(0.16f, 0.42f, 0.14f); // Shade tree dense foliage
    const glm::vec3 dogFur       = glm::vec3(0.72f, 0.50f, 0.26f); // Sleeping dog golden-brown coat
    const glm::vec3 dogDark      = glm::vec3(0.28f, 0.18f, 0.10f); // Dog nose / ears
    const glm::vec3 matStraw     = glm::vec3(0.78f, 0.70f, 0.48f); // Woven straw pet mat
    const glm::vec3 terracotta   = glm::vec3(0.78f, 0.42f, 0.22f); // Clay water jug
    const glm::vec3 clothCream   = glm::vec3(0.88f, 0.85f, 0.76f); // Draped veranda towel
    const glm::vec3 clothBorder  = glm::vec3(0.78f, 0.36f, 0.22f); // Towel accent stripe

    // Core Architecture Dimensions
    const float PILLAR_H = 1.80f; // Stilt height to girder bottom
    const float FLOOR_W  = 8.00f; // Main deck width (X axis, -4.0 to +4.0)
    const float FLOOR_D  = 5.20f; // Main deck depth (Z axis, -2.6 to +2.6)
    const float DECK_Y   = 1.95f; // Finished floor deck surface height
    const float WALL_H   = 2.20f; // Wall height
    const float EAVE_Y   = DECK_Y + WALL_H; // 4.15m (Lower roof eave height)

    // ------------------------------------------------------------------------
    // 1. FOUNDATION STILTS & FOOTINGS (10 Heavy Columns in 2x5 Grid)
    // ------------------------------------------------------------------------
    const float stiltX[5] = { -3.40f, -1.70f, 0.00f, 1.70f, 3.40f };
    const float stiltZ[2] = { -1.80f, 1.80f };

    for (int ix = 0; ix < 5; ++ix)
    {
        for (int iz = 0; iz < 2; ++iz)
        {
            float sx = stiltX[ix];
            float sz = stiltZ[iz];

            // Sample terrain ground elevation under this individual stilt
            glm::vec2 support=terrainSupportRange(baseTransform,sx,sz,.23f,.23f);

            // Square concrete footing
            glm::mat4 foot = glm::translate(baseTransform, glm::vec3(sx, (support.x+support.y)*.5f, sz));
            foot = glm::scale(foot, glm::vec3(0.46f, support.y-support.x, 0.46f));
            drawTransformedCube(shader, cube, foot, concreteCol, 16.0f, 0.10f);

            // Cylindrical wooden stilt column
            float columnBase = support.y;
            float columnH = (DECK_Y - 0.15f) - columnBase;
            if (columnH > 0.1f)
            {
                glm::mat4 col = glm::translate(baseTransform, glm::vec3(sx, columnBase + columnH * 0.5f, sz));
                col = glm::scale(col, glm::vec3(0.24f, columnH, 0.24f));
                drawTransformedFrustum(shader, cylinder, col, woodDark, 20.0f, 0.15f);
            }
        }
    }

    // Clean structural cross-bracing between stilt bays along X (contained under floor)
    for (int ix = 0; ix < 4; ++ix)
    {
        float x1 = stiltX[ix];
        float x2 = stiltX[ix + 1];
        float midX = (x1 + x2) * 0.5f;
        float spanX = x2 - x1;
        float braceH = PILLAR_H * 0.70f;
        float braceL = glm::length(glm::vec2(spanX, braceH));
        float diagAngle = glm::degrees(atan2f(braceH, spanX));

        for (int iz = 0; iz < 2; ++iz)
        {
            float sz = stiltZ[iz];
            // Diagonal 1
            glm::mat4 br1 = glm::translate(baseTransform, glm::vec3(midX, PILLAR_H * 0.50f, sz));
            br1 = glm::rotate(br1, glm::radians(diagAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            br1 = glm::scale(br1, glm::vec3(braceL, 0.07f, 0.07f));
            drawTransformedCube(shader, cube, br1, woodDark, 20.0f, 0.15f);

            // Diagonal 2
            glm::mat4 br2 = glm::translate(baseTransform, glm::vec3(midX, PILLAR_H * 0.50f, sz));
            br2 = glm::rotate(br2, glm::radians(-diagAngle), glm::vec3(0.0f, 0.0f, 1.0f));
            br2 = glm::scale(br2, glm::vec3(braceL, 0.07f, 0.07f));
            drawTransformedCube(shader, cube, br2, woodDark, 20.0f, 0.15f);
        }
    }

    // ------------------------------------------------------------------------
    // 2. UNDERFLOOR STRUCTURAL BEAMS & JOISTS
    // ------------------------------------------------------------------------
    // Longitudinal girder beams
    for (int iz = 0; iz < 2; ++iz)
    {
        glm::mat4 g = glm::translate(baseTransform, glm::vec3(0.0f, DECK_Y - 0.18f, stiltZ[iz]));
        g = glm::scale(g, glm::vec3(FLOOR_W + 0.20f, 0.20f, 0.22f));
        drawTransformedCube(shader, cube, g, woodDark, 24.0f, 0.18f);
    }
    // 9 Transverse floor joists
    for (int i = 0; i < 9; ++i)
    {
        float jx = -3.80f + (float)i * (7.60f / 8.0f);
        glm::mat4 j = glm::translate(baseTransform, glm::vec3(jx, DECK_Y - 0.08f, 0.0f));
        j = glm::scale(j, glm::vec3(0.12f, 0.14f, FLOOR_D + 0.10f));
        drawTransformedCube(shader, cube, j, woodDark, 24.0f, 0.18f);
    }
    // Perimeter edge fascia trim boards
    for (float fz : { -FLOOR_D * 0.5f, FLOOR_D * 0.5f })
    {
        glm::mat4 fascia = glm::translate(baseTransform, glm::vec3(0.0f, DECK_Y - 0.02f, fz));
        fascia = glm::scale(fascia, glm::vec3(FLOOR_W + 0.08f, 0.16f, 0.06f));
        drawTransformedCube(shader, cube, fascia, woodMedium, 24.0f, 0.18f);
    }
    for (float fx : { -FLOOR_W * 0.5f, FLOOR_W * 0.5f })
    {
        glm::mat4 fascia = glm::translate(baseTransform, glm::vec3(fx, DECK_Y - 0.02f, 0.0f));
        fascia = glm::scale(fascia, glm::vec3(0.06f, 0.16f, FLOOR_D + 0.08f));
        drawTransformedCube(shader, cube, fascia, woodMedium, 24.0f, 0.18f);
    }

    // ------------------------------------------------------------------------
    // 3. MAIN RAISED DECK PLATFORM & INDIVIDUAL WOODEN PLANKS
    // ------------------------------------------------------------------------
    glm::mat4 subDeck = glm::translate(baseTransform, glm::vec3(0.0f, DECK_Y - 0.02f, 0.0f));
    subDeck = glm::scale(subDeck, glm::vec3(FLOOR_W - 0.04f, 0.05f, FLOOR_D - 0.04f));
    drawTransformedCube(shader, cube, subDeck, woodDark, 24.0f, 0.15f);

    // 24 individual longitudinal floor planks with subtle alternating tones
    const int numPlanks = 24;
    const float plankSpacing = FLOOR_D / (float)numPlanks;
    for (int p = 0; p < numPlanks; ++p)
    {
        float pz = -FLOOR_D * 0.5f + (float)p * plankSpacing + plankSpacing * 0.5f;
        glm::mat4 plank = glm::translate(baseTransform, glm::vec3(0.0f, DECK_Y + 0.015f, pz));
        plank = glm::scale(plank, glm::vec3(FLOOR_W - 0.06f, 0.03f, plankSpacing * 0.90f));
        glm::vec3 pCol = (p % 2 == 0) ? woodMedium : woodLight;
        drawTransformedCube(shader, cube, plank, pCol, 28.0f, 0.20f);
    }

    // ------------------------------------------------------------------------
    // 4. ENCLOSED ROOM WALLS (Rear section, z = -2.60 to +0.20)
    // ------------------------------------------------------------------------
    const float wallMidY  = DECK_Y + WALL_H * 0.5f; // 3.05m
    const float roomZMin  = -2.60f;
    const float roomZMax  =  0.20f;
    const float roomZMid  = (roomZMin + roomZMax) * 0.5f;
    const float roomDepth = roomZMax - roomZMin; // 2.80m
    const float roomWidth = FLOOR_W - 0.20f;     // 7.80m

    // Structural corner posts of room
    for (float cx : { -roomWidth * 0.5f, roomWidth * 0.5f })
    {
        for (float cz : { roomZMin, roomZMax })
        {
            glm::mat4 cp = glm::translate(baseTransform, glm::vec3(cx, wallMidY, cz));
            cp = glm::scale(cp, glm::vec3(0.18f, WALL_H, 0.18f));
            drawTransformedCube(shader, cube, cp, woodDark, 24.0f, 0.18f);
        }
    }

    // Back Wall (z = roomZMin)
    glm::mat4 backWall = glm::translate(baseTransform, glm::vec3(0.0f, wallMidY, roomZMin));
    backWall = glm::scale(backWall, glm::vec3(roomWidth, WALL_H, 0.08f));
    drawTransformedCube(shader, cube, backWall, bambooWeave, 16.0f, 0.10f);

    // Horizontal reinforcement battens across back wall
    for (float yb : { DECK_Y + 0.60f, DECK_Y + 1.20f, DECK_Y + 1.80f })
    {
        glm::mat4 bat = glm::translate(baseTransform, glm::vec3(0.0f, yb, roomZMin - 0.05f));
        bat = glm::scale(bat, glm::vec3(roomWidth, 0.06f, 0.04f));
        drawTransformedCube(shader, cube, bat, bambooTrim, 20.0f, 0.12f);
    }

    // 2 Back windows with awning shutters propped open with wooden sticks
    for (float wx : { -2.00f, 2.00f })
    {
        // Dark interior void
        glm::mat4 wVoid = glm::translate(baseTransform, glm::vec3(wx, DECK_Y + 1.20f, roomZMin));
        wVoid = glm::scale(wVoid, glm::vec3(0.95f, 0.75f, 0.12f));
        drawTransformedCube(shader, cube, wVoid, glm::vec3(0.10f, 0.08f, 0.05f), 4.0f, 0.02f);

        // Window frame
        glm::mat4 wFrame = glm::translate(baseTransform, glm::vec3(wx, DECK_Y + 1.20f, roomZMin - 0.02f));
        wFrame = glm::scale(wFrame, glm::vec3(1.10f, 0.90f, 0.08f));
        drawTransformedCube(shader, cube, wFrame, woodDark, 24.0f, 0.15f);

        // Propped awning shutter
        glm::mat4 shutter = glm::translate(baseTransform, glm::vec3(wx, DECK_Y + 1.62f, roomZMin - 0.28f));
        shutter = glm::rotate(shutter, glm::radians(38.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shutter = glm::scale(shutter, glm::vec3(0.96f, 0.58f, 0.035f));
        drawTransformedCube(shader, cube, shutter, bambooWeave, 16.0f, 0.10f);

        // Wooden prop stick
        glm::mat4 stick = glm::translate(baseTransform, glm::vec3(wx + 0.35f, DECK_Y + 1.15f, roomZMin - 0.20f));
        stick = glm::rotate(stick, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        stick = glm::scale(stick, glm::vec3(0.03f, 0.65f, 0.03f));
        drawTransformedCube(shader, cube, stick, woodLight, 20.0f, 0.15f);
    }

    // Left Side Wall (x = -roomWidth * 0.5f)
    glm::mat4 leftWall = glm::translate(baseTransform, glm::vec3(-roomWidth * 0.5f, wallMidY, roomZMid));
    leftWall = glm::scale(leftWall, glm::vec3(0.08f, WALL_H, roomDepth));
    drawTransformedCube(shader, cube, leftWall, bambooWeave, 16.0f, 0.10f);

    // Left window with propped shutter
    {
        glm::mat4 wFrame = glm::translate(baseTransform, glm::vec3(-roomWidth * 0.5f - 0.02f, DECK_Y + 1.20f, roomZMid));
        wFrame = glm::scale(wFrame, glm::vec3(0.08f, 0.85f, 0.95f));
        drawTransformedCube(shader, cube, wFrame, woodDark, 24.0f, 0.15f);

        glm::mat4 shutter = glm::translate(baseTransform, glm::vec3(-roomWidth * 0.5f - 0.25f, DECK_Y + 1.60f, roomZMid));
        shutter = glm::rotate(shutter, glm::radians(-35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        shutter = glm::scale(shutter, glm::vec3(0.035f, 0.55f, 0.92f));
        drawTransformedCube(shader, cube, shutter, bambooWeave, 16.0f, 0.10f);
    }

    // Right Side Wall (x = roomWidth * 0.5f) with secondary exit doorway & full structural framing
    // Back segment: from z = roomZMin (-2.60m) to door back jamb (-1.65m) -> depth 0.95m, center -2.125m
    glm::mat4 rightWallB = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f, wallMidY, -2.125f));
    rightWallB = glm::scale(rightWallB, glm::vec3(0.08f, WALL_H, 0.95f));
    drawTransformedCube(shader, cube, rightWallB, bambooWeave, 16.0f, 0.10f);

    // Window on right back wall with propped awning shutter
    {
        glm::mat4 rwVoid = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f, DECK_Y + 1.20f, -2.125f));
        rwVoid = glm::scale(rwVoid, glm::vec3(0.10f, 0.70f, 0.65f));
        drawTransformedCube(shader, cube, rwVoid, glm::vec3(0.10f, 0.08f, 0.05f), 4.0f, 0.02f);

        glm::mat4 rwFrame = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f + 0.02f, DECK_Y + 1.20f, -2.125f));
        rwFrame = glm::scale(rwFrame, glm::vec3(0.08f, 0.82f, 0.75f));
        drawTransformedCube(shader, cube, rwFrame, woodDark, 24.0f, 0.15f);

        glm::mat4 rwShutter = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f + 0.22f, DECK_Y + 1.55f, -2.125f));
        rwShutter = glm::rotate(rwShutter, glm::radians(35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        rwShutter = glm::scale(rwShutter, glm::vec3(0.035f, 0.52f, 0.72f));
        drawTransformedCube(shader, cube, rwShutter, bambooWeave, 16.0f, 0.10f);

        glm::mat4 rwStick = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f + 0.16f, DECK_Y + 1.15f, -2.125f));
        rwStick = glm::rotate(rwStick, glm::radians(-40.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        rwStick = glm::scale(rwStick, glm::vec3(0.03f, 0.55f, 0.03f));
        drawTransformedCube(shader, cube, rwStick, woodLight, 20.0f, 0.15f);
    }

    // Front segment: from door front jamb (-0.75m) to roomZMax (+0.20m) -> depth 0.95m, center -0.275m
    glm::mat4 rightWallF = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f, wallMidY, -0.275f));
    rightWallF = glm::scale(rightWallF, glm::vec3(0.08f, WALL_H, 0.95f));
    drawTransformedCube(shader, cube, rightWallF, bambooWeave, 16.0f, 0.10f);

    // Structural Door Jambs (framing the 0.90m doorway opening at z = roomZMid = -1.20m)
    for (float jz : { roomZMid - 0.45f, roomZMid + 0.45f })
    {
        glm::mat4 dJamb = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f, DECK_Y + 1.00f, jz));
        dJamb = glm::scale(dJamb, glm::vec3(0.12f, 2.00f, 0.08f));
        drawTransformedCube(shader, cube, dJamb, woodDark, 24.0f, 0.18f);
    }

    // Door Header Lintel Beam (top of door opening at y = DECK_Y + 2.00m)
    glm::mat4 rightLintel = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f, DECK_Y + 2.00f, roomZMid));
    rightLintel = glm::scale(rightLintel, glm::vec3(0.14f, 0.12f, 0.98f));
    drawTransformedCube(shader, cube, rightLintel, woodDark, 24.0f, 0.18f);

    // Wall panel above door header (sealing gap up to roof eave)
    glm::mat4 rightWallTop = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f, DECK_Y + 2.12f, roomZMid));
    rightWallTop = glm::scale(rightWallTop, glm::vec3(0.08f, 0.12f, 0.90f));
    drawTransformedCube(shader, cube, rightWallTop, bambooWeave, 16.0f, 0.10f);

    // Threshold sill board at floor level
    glm::mat4 rightSill = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f + 0.03f, DECK_Y + 0.015f, roomZMid));
    rightSill = glm::scale(rightSill, glm::vec3(0.14f, 0.04f, 0.96f));
    drawTransformedCube(shader, cube, rightSill, woodMedium, 22.0f, 0.16f);

    // Front Dividing Wall (z = roomZMax)
    float sideWallW = (roomWidth - 1.40f) * 0.5f;
    glm::mat4 fWallL = glm::translate(baseTransform, glm::vec3(-roomWidth * 0.5f + sideWallW * 0.5f, wallMidY, roomZMax));
    fWallL = glm::scale(fWallL, glm::vec3(sideWallW, WALL_H, 0.08f));
    drawTransformedCube(shader, cube, fWallL, bambooWeave, 16.0f, 0.10f);

    glm::mat4 fWallR = glm::translate(baseTransform, glm::vec3(roomWidth * 0.5f - sideWallW * 0.5f, wallMidY, roomZMax));
    fWallR = glm::scale(fWallR, glm::vec3(sideWallW, WALL_H, 0.08f));
    drawTransformedCube(shader, cube, fWallR, bambooWeave, 16.0f, 0.10f);

    // Center doorway posts
    for (float dx : { -0.70f, 0.70f })
    {
        glm::mat4 dPost = glm::translate(baseTransform, glm::vec3(dx, wallMidY, roomZMax));
        dPost = glm::scale(dPost, glm::vec3(0.12f, WALL_H, 0.12f));
        drawTransformedCube(shader, cube, dPost, woodDark, 24.0f, 0.18f);
    }
    glm::mat4 dLintel = glm::translate(baseTransform, glm::vec3(0.0f, DECK_Y + 2.05f, roomZMax));
    dLintel = glm::scale(dLintel, glm::vec3(1.50f, 0.15f, 0.14f));
    drawTransformedCube(shader, cube, dLintel, woodDark, 24.0f, 0.18f);

    // ------------------------------------------------------------------------
    // 5. OPEN-AIR FRONT VERANDA / PORCH (z = +0.20 to +2.60)
    // ------------------------------------------------------------------------
    // 4 Heavy cylindrical veranda posts supporting roof eave beam along z = +2.55m
    const float verandaPostX[4] = { -3.80f, -1.30f, 1.30f, 3.80f };
    for (int i = 0; i < 4; ++i)
    {
        glm::mat4 vp = glm::translate(baseTransform, glm::vec3(verandaPostX[i], wallMidY, 2.55f));
        vp = glm::scale(vp, glm::vec3(0.20f, WALL_H, 0.20f));
        drawTransformedFrustum(shader, cylinder, vp, woodDark, 24.0f, 0.18f);

        // Angled knee braces
        if (i > 0 && i < 3)
        {
            glm::mat4 kb1 = glm::translate(baseTransform, glm::vec3(verandaPostX[i] + 0.22f, EAVE_Y - 0.25f, 2.55f));
            kb1 = glm::rotate(kb1, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            kb1 = glm::scale(kb1, glm::vec3(0.07f, 0.45f, 0.07f));
            drawTransformedCube(shader, cube, kb1, woodDark, 24.0f, 0.18f);

            glm::mat4 kb2 = glm::translate(baseTransform, glm::vec3(verandaPostX[i] - 0.22f, EAVE_Y - 0.25f, 2.55f));
            kb2 = glm::rotate(kb2, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            kb2 = glm::scale(kb2, glm::vec3(0.07f, 0.45f, 0.07f));
            drawTransformedCube(shader, cube, kb2, woodDark, 24.0f, 0.18f);
        }
    }

    // Top eave lintel beam over veranda posts
    glm::mat4 vBeam = glm::translate(baseTransform, glm::vec3(0.0f, EAVE_Y, 2.55f));
    vBeam = glm::scale(vBeam, glm::vec3(FLOOR_W + 0.10f, 0.16f, 0.18f));
    drawTransformedCube(shader, cube, vBeam, woodDark, 24.0f, 0.18f);

    // Veranda Balustrades & Railings
    const float railTopY  = DECK_Y + 0.88f;
    const float railBotY  = DECK_Y + 0.18f;
    const float railMidY  = (railTopY + railBotY) * 0.5f;
    const float balHeight = railTopY - railBotY;

    // Center-left front railing (leaving left side open for stairs at x = -3.85 to -2.60)
    glm::mat4 rTopC = glm::translate(baseTransform, glm::vec3(0.0f, railTopY, 2.55f));
    rTopC = glm::scale(rTopC, glm::vec3(2.45f, 0.07f, 0.09f));
    drawTransformedCube(shader, cube, rTopC, woodLight, 24.0f, 0.18f);

    glm::mat4 rBotC = glm::translate(baseTransform, glm::vec3(0.0f, railBotY, 2.55f));
    rBotC = glm::scale(rBotC, glm::vec3(2.45f, 0.05f, 0.07f));
    drawTransformedCube(shader, cube, rBotC, woodMedium, 24.0f, 0.18f);

    for (int b = 0; b < 6; ++b)
    {
        float bx = -1.05f + (float)b * (2.10f / 5.0f);
        glm::mat4 bal = glm::translate(baseTransform, glm::vec3(bx, railMidY, 2.55f));
        bal = glm::scale(bal, glm::vec3(0.05f, balHeight, 0.05f));
        drawTransformedCube(shader, cube, bal, woodMedium, 20.0f, 0.15f);
    }

    // Right front railing
    glm::mat4 rTopR = glm::translate(baseTransform, glm::vec3(2.55f, railTopY, 2.55f));
    rTopR = glm::scale(rTopR, glm::vec3(2.35f, 0.07f, 0.09f));
    drawTransformedCube(shader, cube, rTopR, woodLight, 24.0f, 0.18f);

    glm::mat4 rBotR = glm::translate(baseTransform, glm::vec3(2.55f, railBotY, 2.55f));
    rBotR = glm::scale(rBotR, glm::vec3(2.35f, 0.05f, 0.07f));
    drawTransformedCube(shader, cube, rBotR, woodMedium, 24.0f, 0.18f);

    for (int b = 0; b < 6; ++b)
    {
        float bx = 1.50f + (float)b * (2.10f / 5.0f);
        glm::mat4 bal = glm::translate(baseTransform, glm::vec3(bx, railMidY, 2.55f));
        bal = glm::scale(bal, glm::vec3(0.05f, balHeight, 0.05f));
        drawTransformedCube(shader, cube, bal, woodMedium, 20.0f, 0.15f);
    }

    // Right side veranda railing
    glm::mat4 rTopSideR = glm::translate(baseTransform, glm::vec3(3.80f, railTopY, 1.40f));
    rTopSideR = glm::scale(rTopSideR, glm::vec3(0.09f, 0.07f, 2.20f));
    drawTransformedCube(shader, cube, rTopSideR, woodLight, 24.0f, 0.18f);

    glm::mat4 rBotSideR = glm::translate(baseTransform, glm::vec3(3.80f, railBotY, 1.40f));
    rBotSideR = glm::scale(rBotSideR, glm::vec3(0.07f, 0.05f, 2.20f));
    drawTransformedCube(shader, cube, rBotSideR, woodMedium, 24.0f, 0.18f);

    for (int b = 0; b < 6; ++b)
    {
        float bz = 0.45f + (float)b * (1.90f / 5.0f);
        glm::mat4 bal = glm::translate(baseTransform, glm::vec3(3.80f, railMidY, bz));
        bal = glm::scale(bal, glm::vec3(0.05f, balHeight, 0.05f));
        drawTransformedCube(shader, cube, bal, woodMedium, 20.0f, 0.15f);
    }

    // ------------------------------------------------------------------------
    // 6. SIGNATURE TRADITIONAL PITCHED THATCH ROOF (ACCURATE WATER-TIGHT SLOPES)
    // ------------------------------------------------------------------------
    const float OVERHANG_Z   = 0.90f; // Eaves overhang beyond front/back walls
    const float OVERHANG_X   = 0.80f; // Overhang beyond left/right gable walls
    const float ROOF_W       = FLOOR_W + 2.0f * OVERHANG_X; // 9.60m
    const float HALF_ROOF_D  = (FLOOR_D * 0.5f) + OVERHANG_Z; // 2.60 + 0.90 = 3.50m
    const float TOTAL_ROOF_D = HALF_ROOF_D * 2.0f; // 7.00m
    const float ROOF_PITCH   = 36.0f; // Degrees
    const float ROOF_RISE    = HALF_ROOF_D * tanf(glm::radians(ROOF_PITCH)); // 2.54m
    const float PEAK_Y       = EAVE_Y + ROOF_RISE; // 4.15 + 2.54 = 6.69m
    const float SLOPE_L      = HALF_ROOF_D / cosf(glm::radians(ROOF_PITCH)); // 4.33m

    const float roofMidY     = (PEAK_Y + EAVE_Y) * 0.5f; // 5.42m
    const float roofMidZ     = HALF_ROOF_D * 0.5f;       // 1.75m

    // Heavy ridge log along peak
    glm::mat4 ridgeLog = glm::translate(baseTransform, glm::vec3(0.0f, PEAK_Y, 0.0f));
    ridgeLog = glm::rotate(ridgeLog, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ridgeLog = glm::scale(ridgeLog, glm::vec3(0.24f, ROOF_W + 0.20f, 0.24f));
    drawTransformedFrustum(shader, cylinder, ridgeLog, woodDark, 24.0f, 0.15f);

    // Thick woven thatch ridge cap crowning the peak seam
    glm::mat4 ridgeCap = glm::translate(baseTransform, glm::vec3(0.0f, PEAK_Y + 0.12f, 0.0f));
    ridgeCap = glm::scale(ridgeCap, glm::vec3(ROOF_W + 0.15f, 0.26f, 0.55f));
    drawTransformedCube(shader, cube, ridgeCap, thatchFringe, 12.0f, 0.06f);

    // Front Roof Slope (+Z side: slopes DOWN from peak at z=0 to eave at z=+HALF_ROOF_D)
    {
        glm::mat4 frontSlope = glm::translate(baseTransform, glm::vec3(0.0f, roofMidY, roofMidZ));
        frontSlope = glm::rotate(frontSlope, glm::radians(ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
        frontSlope = glm::scale(frontSlope, glm::vec3(ROOF_W, 0.22f, SLOPE_L));
        drawTransformedCube(shader, cube, frontSlope, thatchRoof, 10.0f, 0.06f);

        // Tiered / stepped thatch fringe strips along lower front eave
        for (int t = 0; t < 3; ++t)
        {
            float fOffset = (float)t * 0.30f;
            glm::mat4 fr = glm::translate(baseTransform, glm::vec3(0.0f, roofMidY, roofMidZ));
            fr = glm::rotate(fr, glm::radians(ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
            fr = glm::translate(fr, glm::vec3(0.0f, 0.07f - (float)t * 0.025f, SLOPE_L * 0.5f - fOffset - 0.15f));
            fr = glm::scale(fr, glm::vec3(ROOF_W + 0.06f, 0.09f, 0.35f));
            drawTransformedCube(shader, cube, fr, thatchFringe, 8.0f, 0.04f);
        }
    }

    // Back Roof Slope (-Z side: slopes DOWN from peak at z=0 to eave at z=-HALF_ROOF_D)
    {
        glm::mat4 backSlope = glm::translate(baseTransform, glm::vec3(0.0f, roofMidY, -roofMidZ));
        backSlope = glm::rotate(backSlope, glm::radians(-ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
        backSlope = glm::scale(backSlope, glm::vec3(ROOF_W, 0.22f, SLOPE_L));
        drawTransformedCube(shader, cube, backSlope, thatchRoof, 10.0f, 0.06f);

        // Tiered / stepped thatch fringe strips along lower back eave
        for (int t = 0; t < 3; ++t)
        {
            float fOffset = (float)t * 0.30f;
            glm::mat4 fr = glm::translate(baseTransform, glm::vec3(0.0f, roofMidY, -roofMidZ));
            fr = glm::rotate(fr, glm::radians(-ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
            fr = glm::translate(fr, glm::vec3(0.0f, 0.07f - (float)t * 0.025f, -SLOPE_L * 0.5f + fOffset + 0.15f));
            fr = glm::scale(fr, glm::vec3(ROOF_W + 0.06f, 0.09f, 0.35f));
            drawTransformedCube(shader, cube, fr, thatchFringe, 8.0f, 0.04f);
        }
    }

    // Front and Back lower eave fascia boards
    for (float fz : { -HALF_ROOF_D, HALF_ROOF_D })
    {
        glm::mat4 eFascia = glm::translate(baseTransform, glm::vec3(0.0f, EAVE_Y - 0.04f, fz));
        eFascia = glm::scale(eFascia, glm::vec3(ROOF_W + 0.05f, 0.14f, 0.08f));
        drawTransformedCube(shader, cube, eFascia, woodDark, 20.0f, 0.15f);
    }

    // Exposed structural rafters underneath both slopes (9 rafter pairs along X)
    for (int i = 0; i < 9; ++i)
    {
        float rx = -4.20f + (float)i * (8.40f / 8.0f);

        // Front rafter
        glm::mat4 rafF = glm::translate(baseTransform, glm::vec3(rx, roofMidY - 0.12f, roofMidZ));
        rafF = glm::rotate(rafF, glm::radians(ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
        rafF = glm::scale(rafF, glm::vec3(0.09f, 0.12f, SLOPE_L));
        drawTransformedCube(shader, cube, rafF, woodDark, 20.0f, 0.15f);

        // Back rafter
        glm::mat4 rafB = glm::translate(baseTransform, glm::vec3(rx, roofMidY - 0.12f, -roofMidZ));
        rafB = glm::rotate(rafB, glm::radians(-ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
        rafB = glm::scale(rafB, glm::vec3(0.09f, 0.12f, SLOPE_L));
        drawTransformedCube(shader, cube, rafB, woodDark, 20.0f, 0.15f);
    }

    // Triangular Gable Walls (Left and Right Ends) - Mathematical Watertight Fit!
    for (float gx : { -roomWidth * 0.5f, roomWidth * 0.5f })
    {
        // Triangular gable wall using PrismMesh rotated 90 deg around Y
        glm::mat4 gable = glm::translate(baseTransform, glm::vec3(gx, roofMidY, 0.0f));
        gable = glm::rotate(gable, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        gable = glm::scale(gable, glm::vec3(TOTAL_ROOF_D, ROOF_RISE, 0.10f));
        drawTransformedPrism(shader, prism, gable, bambooWeave, 16.0f, 0.10f);

        // Sloping bargeboard trims framing the triangular roof edges
        glm::mat4 trimF = glm::translate(baseTransform, glm::vec3(gx * 1.01f, roofMidY, roofMidZ));
        trimF = glm::rotate(trimF, glm::radians(ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
        trimF = glm::scale(trimF, glm::vec3(0.12f, 0.14f, SLOPE_L));
        drawTransformedCube(shader, cube, trimF, woodDark, 20.0f, 0.15f);

        glm::mat4 trimB = glm::translate(baseTransform, glm::vec3(gx * 1.01f, roofMidY, -roofMidZ));
        trimB = glm::rotate(trimB, glm::radians(-ROOF_PITCH), glm::vec3(1.0f, 0.0f, 0.0f));
        trimB = glm::scale(trimB, glm::vec3(0.12f, 0.14f, SLOPE_L));
        drawTransformedCube(shader, cube, trimB, woodDark, 20.0f, 0.15f);

        // Attic triangular louvered vent in gable center
        glm::mat4 vent = glm::translate(baseTransform, glm::vec3(gx + (gx < 0 ? -0.06f : 0.06f), EAVE_Y + ROOF_RISE * 0.40f, 0.0f));
        vent = glm::scale(vent, glm::vec3(0.04f, 0.60f, 0.75f));
        drawTransformedCube(shader, cube, vent, woodDark, 20.0f, 0.15f);
    }

    // ------------------------------------------------------------------------
    // 7. LEFT MAIN ENTRANCE STAIRCASE (10 Steps, Grounded & Aligned)
    // Extends from ground (x=-6.45m) up to veranda deck (x=-3.85m, y=1.95m, z=1.40m)
    // ------------------------------------------------------------------------
    const int   stairSteps = 10;
    const float runX       = 2.60f;
    const glm::vec2 leftSupport=terrainSupportRange(baseTransform,-3.85f-runX-.15f,1.40f,.35f,.625f,.08f);
    const float leftGround=leftSupport.y;
    const float riseY      = DECK_Y-leftGround;
    const float stepH      = riseY / (float)stairSteps; // 0.195m
    const float stepD      = runX / (float)stairSteps;  // 0.260m
    const float stepW      = 1.00f;
    const float stairAng   = glm::degrees(atan2f(riseY, runX)); // +36.87 deg
    const float stairL     = sqrtf(runX * runX + riseY * riseY); // 3.25m
    const float stairMidX  = -3.85f - runX * 0.5f; // -5.15m
    const float stairMidY  = leftGround+riseY * 0.5f - 0.06f;
    const float stairZ     = 1.40f;

    // Concrete Ground Landing Pad at base of left staircase
    glm::mat4 lPad = glm::translate(baseTransform, glm::vec3(-3.85f - runX - 0.15f, (leftSupport.x+leftSupport.y)*.5f, stairZ));
    lPad = glm::scale(lPad, glm::vec3(0.70f, leftSupport.y-leftSupport.x, stepW + 0.25f));
    drawTransformedCube(shader, cube, lPad, concreteCol, 16.0f, 0.10f);

    // 10 solid step treads nestled firmly between the stringers
    for (int s = 0; s < stairSteps; ++s)
    {
        float sy = leftGround+(float)(s+1)*stepH-.025f;
        float sx = -3.85f - runX + (float)s * stepD + stepD * 0.5f;

        // Tread
        glm::mat4 step = glm::translate(baseTransform, glm::vec3(sx, sy, stairZ));
        step = glm::scale(step, glm::vec3(stepD * 1.05f, 0.05f, stepW));
        drawTransformedCube(shader, cube, step, woodLight, 24.0f, 0.18f);

        // Riser
        glm::mat4 riser = glm::translate(baseTransform, glm::vec3(sx + stepD * 0.48f, sy - stepH * 0.5f, stairZ));
        riser = glm::scale(riser, glm::vec3(0.04f, stepH, stepW));
        drawTransformedCube(shader, cube, riser, woodDark, 20.0f, 0.15f);
    }

    // Heavy angled side stringers (Left & Right of steps) - Positive slope upwards to deck!
    for (float sideOffset : { -stepW * 0.52f, stepW * 0.52f })
    {
        glm::mat4 str = glm::translate(baseTransform, glm::vec3(stairMidX, stairMidY, stairZ + sideOffset));
        str = glm::rotate(str, glm::radians(stairAng), glm::vec3(0.0f, 0.0f, 1.0f));
        str = glm::scale(str, glm::vec3(stairL, 0.18f, 0.08f));
        drawTransformedCube(shader, cube, str, woodDark, 24.0f, 0.18f);

        // Handrail posts mounted vertically on stringer
        for (int p = 0; p < 4; ++p)
        {
            float t = (float)p / 3.0f;
            float px = -3.85f - runX * (1.0f - t);
            float py = leftGround+riseY * t + 0.42f;
            glm::mat4 hPost = glm::translate(baseTransform, glm::vec3(px, py, stairZ + sideOffset));
            hPost = glm::scale(hPost, glm::vec3(0.07f, 0.85f, 0.07f));
            drawTransformedCube(shader, cube, hPost, woodMedium, 20.0f, 0.15f);
        }

        // Continuous smooth top handrail beam
        glm::mat4 hRail = glm::translate(baseTransform, glm::vec3(stairMidX, stairMidY + 0.82f, stairZ + sideOffset));
        hRail = glm::rotate(hRail, glm::radians(stairAng), glm::vec3(0.0f, 0.0f, 1.0f));
        hRail = glm::scale(hRail, glm::vec3(stairL + 0.20f, 0.07f, 0.09f));
        drawTransformedCube(shader, cube, hRail, woodLight, 24.0f, 0.18f);

        // Mid rail
        glm::mat4 mRail = glm::translate(baseTransform, glm::vec3(stairMidX, stairMidY + 0.42f, stairZ + sideOffset));
        mRail = glm::rotate(mRail, glm::radians(stairAng), glm::vec3(0.0f, 0.0f, 1.0f));
        mRail = glm::scale(mRail, glm::vec3(stairL + 0.10f, 0.04f, 0.06f));
        drawTransformedCube(shader, cube, mRail, woodMedium, 20.0f, 0.15f);
    }

    // ------------------------------------------------------------------------
    // 8. RIGHT SECONDARY STAIRCASE (8 Solid Steps, Dual Stringers, Balusters & Rails)
    // Extends down from side doorway (x=3.85m, y=1.95m, z=-1.20m) to ground (x=6.17m, y=0.0m)
    // ------------------------------------------------------------------------
    const int   rSteps    = 8;
    const float rRunX     = 2.32f;
    const glm::vec2 rightSupport=terrainSupportRange(baseTransform,roomWidth*.5f+rRunX+.12f,roomZMid,.35f,.575f,.08f);
    const float rightGround=rightSupport.y;
    const float rRiseY    = DECK_Y-rightGround;
    const float rStepH    = rRiseY / (float)rSteps; // 0.24375m
    const float rStepD    = rRunX / (float)rSteps;  // 0.290m
    const float rStepW    = 0.90f;
    const float rDoorX    = roomWidth * 0.5f;       // 3.85m
    const float rGroundX  = rDoorX + rRunX;          // 6.17m
    const float rStairL   = sqrtf(rRunX * rRunX + rRiseY * rRiseY); // 3.031m
    const float rStairAng = glm::degrees(atan2f(rRiseY, rRunX));    // 40.05 deg
    const float rMidX     = (rDoorX + rGroundX) * 0.5f;             // 5.01m
    const float rMidY     = rightGround+rRiseY * 0.5f - 0.06f;
    const float rStairZ   = roomZMid;                               // -1.20m

    // Concrete Ground Landing Pad at base of right staircase
    glm::mat4 rPad = glm::translate(baseTransform, glm::vec3(rGroundX + 0.12f, (rightSupport.x+rightSupport.y)*.5f, rStairZ));
    rPad = glm::scale(rPad, glm::vec3(0.70f, rightSupport.y-rightSupport.x, rStepW + 0.25f));
    drawTransformedCube(shader, cube, rPad, concreteCol, 16.0f, 0.10f);

    // 8 solid step treads with vertical risers nestled firmly between stringers
    for (int s = 0; s < rSteps; ++s)
    {
        float sy = rightGround+(float)(s+1)*rStepH-.025f;
        float sx = rGroundX - (float)s * rStepD - rStepD * 0.5f;

        // Tread
        glm::mat4 step = glm::translate(baseTransform, glm::vec3(sx, sy, rStairZ));
        step = glm::scale(step, glm::vec3(rStepD * 1.05f, 0.05f, rStepW));
        drawTransformedCube(shader, cube, step, woodLight, 24.0f, 0.18f);

        // Riser (uphill vertical face of tread)
        glm::mat4 riser = glm::translate(baseTransform, glm::vec3(sx - rStepD * 0.48f, sy - rStepH * 0.5f, rStairZ));
        riser = glm::scale(riser, glm::vec3(0.04f, rStepH, rStepW));
        drawTransformedCube(shader, cube, riser, woodDark, 20.0f, 0.15f);
    }

    // Heavy angled side stringers (Left & Right of steps) - Sloping down towards +X
    for (float sideOffset : { -rStepW * 0.52f, rStepW * 0.52f })
    {
        // Side stringer beam
        glm::mat4 str = glm::translate(baseTransform, glm::vec3(rMidX, rMidY, rStairZ + sideOffset));
        str = glm::rotate(str, glm::radians(-rStairAng), glm::vec3(0.0f, 0.0f, 1.0f));
        str = glm::scale(str, glm::vec3(rStairL, 0.18f, 0.08f));
        drawTransformedCube(shader, cube, str, woodDark, 24.0f, 0.18f);

        // Handrail posts mounted vertically along stringer
        for (int p = 0; p < 4; ++p)
        {
            float t = (float)p / 3.0f;
            float px = rDoorX + rRunX * (1.0f - t);
            float py = rightGround+rRiseY * t + 0.42f;
            glm::mat4 hPost = glm::translate(baseTransform, glm::vec3(px, py, rStairZ + sideOffset));
            hPost = glm::scale(hPost, glm::vec3(0.07f, 0.85f, 0.07f));
            drawTransformedCube(shader, cube, hPost, woodMedium, 20.0f, 0.15f);
        }

        // Continuous smooth top handrail beam
        glm::mat4 hRail = glm::translate(baseTransform, glm::vec3(rMidX, rMidY + 0.82f, rStairZ + sideOffset));
        hRail = glm::rotate(hRail, glm::radians(-rStairAng), glm::vec3(0.0f, 0.0f, 1.0f));
        hRail = glm::scale(hRail, glm::vec3(rStairL + 0.20f, 0.07f, 0.09f));
        drawTransformedCube(shader, cube, hRail, woodLight, 24.0f, 0.18f);

        // Mid rail
        glm::mat4 mRail = glm::translate(baseTransform, glm::vec3(rMidX, rMidY + 0.42f, rStairZ + sideOffset));
        mRail = glm::rotate(mRail, glm::radians(-rStairAng), glm::vec3(0.0f, 0.0f, 1.0f));
        mRail = glm::scale(mRail, glm::vec3(rStairL + 0.10f, 0.04f, 0.06f));
        drawTransformedCube(shader, cube, mRail, woodMedium, 20.0f, 0.15f);
    }

    // ------------------------------------------------------------------------
    // 9. VERANDA FURNITURE & PROPS (Matching Photo Details)
    // ------------------------------------------------------------------------
    // Wooden Veranda Lounge Chair
    {
        const glm::vec3 chairPos = glm::vec3(0.20f, DECK_Y + 0.20f, 1.50f);
        // Seat
        glm::mat4 cSeat = glm::translate(baseTransform, chairPos);
        cSeat = glm::scale(cSeat, glm::vec3(0.65f, 0.04f, 0.70f));
        drawTransformedCube(shader, cube, cSeat, woodLight, 24.0f, 0.18f);

        // 4 Legs
        for (float lx : { -0.28f, 0.28f })
        {
            for (float lz : { -0.30f, 0.30f })
            {
                glm::mat4 cleg = glm::translate(baseTransform, chairPos + glm::vec3(lx, -0.10f, lz));
                cleg = glm::scale(cleg, glm::vec3(0.05f, 0.20f, 0.05f));
                drawTransformedCube(shader, cube, cleg, woodDark, 20.0f, 0.15f);
            }
        }
        // Reclined backrest
        glm::mat4 cBack = glm::translate(baseTransform, chairPos + glm::vec3(0.0f, 0.34f, -0.30f));
        cBack = glm::rotate(cBack, glm::radians(-18.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        cBack = glm::scale(cBack, glm::vec3(0.65f, 0.65f, 0.04f));
        drawTransformedCube(shader, cube, cBack, woodLight, 24.0f, 0.18f);

        // Armrests
        for (float ax : { -0.32f, 0.32f })
        {
            glm::mat4 arm = glm::translate(baseTransform, chairPos + glm::vec3(ax, 0.18f, -0.05f));
            arm = glm::scale(arm, glm::vec3(0.06f, 0.03f, 0.60f));
            drawTransformedCube(shader, cube, arm, woodLight, 24.0f, 0.18f);

            glm::mat4 armP = glm::translate(baseTransform, chairPos + glm::vec3(ax, 0.09f, 0.22f));
            armP = glm::scale(armP, glm::vec3(0.04f, 0.18f, 0.04f));
            drawTransformedCube(shader, cube, armP, woodDark, 20.0f, 0.15f);
        }
    }

    // Small low wooden side table next to chair
    {
        glm::mat4 tTop = glm::translate(baseTransform, glm::vec3(1.05f, DECK_Y + 0.22f, 1.50f));
        tTop = glm::scale(tTop, glm::vec3(0.45f, 0.03f, 0.45f));
        drawTransformedCube(shader, cube, tTop, woodMedium, 24.0f, 0.18f);

        for (float tx : { -0.18f, 0.18f })
        {
            for (float tz : { -0.18f, 0.18f })
            {
                glm::mat4 tLeg = glm::translate(baseTransform, glm::vec3(1.05f + tx, DECK_Y + 0.11f, 1.50f + tz));
                tLeg = glm::scale(tLeg, glm::vec3(0.04f, 0.22f, 0.04f));
                drawTransformedCube(shader, cube, tLeg, woodDark, 20.0f, 0.15f);
            }
        }
    }

    // Terracotta water jar by doorway
    {
        const glm::vec3 jarPos = glm::vec3(-1.60f, DECK_Y + 0.28f, 1.90f);
        glm::mat4 jarBody = glm::translate(baseTransform, jarPos);
        jarBody = glm::scale(jarBody, glm::vec3(0.48f, 0.54f, 0.48f));
        drawTransformedSphere(shader, sphere, jarBody, terracotta, 28.0f, 0.25f);

        glm::mat4 jarNeck = glm::translate(baseTransform, jarPos + glm::vec3(0.0f, 0.30f, 0.0f));
        jarNeck = glm::scale(jarNeck, glm::vec3(0.24f, 0.14f, 0.24f));
        drawTransformedFrustum(shader, cylinder, jarNeck, terracotta, 28.0f, 0.25f);

        glm::mat4 jarRim = glm::translate(baseTransform, jarPos + glm::vec3(0.0f, 0.37f, 0.0f));
        jarRim = glm::scale(jarRim, glm::vec3(0.32f, 0.04f, 0.32f));
        drawTransformedFrustum(shader, cylinder, jarRim, terracotta, 28.0f, 0.25f);
    }

    // Draped tropical cloth / sarong towel on front railing
    {
        glm::mat4 cloth = glm::translate(baseTransform, glm::vec3(2.10f, railTopY - 0.20f, 2.55f));
        cloth = glm::scale(cloth, glm::vec3(0.60f, 0.46f, 0.11f));
        drawTransformedCube(shader, cube, cloth, clothCream, 16.0f, 0.10f);

        glm::mat4 stripe = glm::translate(baseTransform, glm::vec3(2.10f, railTopY - 0.38f, 2.55f));
        stripe = glm::scale(stripe, glm::vec3(0.60f, 0.08f, 0.12f));
        drawTransformedCube(shader, cube, stripe, clothBorder, 16.0f, 0.10f);
    }

    // ------------------------------------------------------------------------
    // 10. RAISED TIMBER PLANTER BED WITH REALISTIC MULTI-FROND TROPICAL PLANTS
    // Placed in front of the house at ground level (x=0.50m, z=3.80m)
    // ------------------------------------------------------------------------
    {
        const glm::vec3 pPos = glm::vec3(0.50f, 0.18f, 3.80f);
        const float pW = 4.40f;
        const float pD = 1.30f;
        const float pH = 0.36f;

        // Timber planter box frame
        glm::mat4 pBox = glm::translate(baseTransform, pPos);
        pBox = glm::scale(pBox, glm::vec3(pW, pH, pD));
        drawTransformedCube(shader, cube, pBox, woodDark, 20.0f, 0.12f);

        // Rich dark loam soil inside
        glm::mat4 pSoil = glm::translate(baseTransform, pPos + glm::vec3(0.0f, 0.10f, 0.0f));
        pSoil = glm::scale(pSoil, glm::vec3(pW - 0.20f, 0.18f, pD - 0.20f));
        drawTransformedCube(shader, cube, pSoil, soilCol, 6.0f, 0.04f);

        // 5 Lush Tropical Plant Clusters with realistic arching broadleaf fronds!
        const float pClusterX[5] = { -1.50f, -0.75f, 0.00f, 0.85f, 1.55f };
        const float pClusterZ[5] = {  0.10f, -0.15f, 0.15f, -0.10f, 0.12f };
        const float pClusterS[5] = {  0.75f,  0.85f, 0.95f,  0.80f, 0.70f };

        for (int c = 0; c < 5; ++c)
        {
            glm::vec3 cPos = pPos + glm::vec3(pClusterX[c], 0.22f, pClusterZ[c]);
            float cScale = pClusterS[c];

            // 7 Arching leaves radiating in a circle
            const int numLeaves = 7;
            for (int l = 0; l < numLeaves; ++l)
            {
                float yaw = (float)l * (360.0f / (float)numLeaves) + (float)c * 25.0f;
                float droop = 28.0f + (float)(l % 3) * 6.0f;
                float leafL = 0.55f * cScale;

                glm::mat4 lf = glm::translate(baseTransform, cPos);
                lf = glm::rotate(lf, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
                lf = glm::rotate(lf, glm::radians(droop), glm::vec3(1.0f, 0.0f, 0.0f));
                lf = glm::translate(lf, glm::vec3(0.0f, 0.0f, leafL * 0.5f));
                lf = glm::scale(lf, glm::vec3(0.14f * cScale, 0.018f, leafL));
                glm::vec3 lCol = (l % 2 == 0) ? leafDark : leafBright;
                drawTransformedCube(shader, cube, lf, lCol, 16.0f, 0.15f);
            }

            // Central shoot cone
            glm::mat4 shoot = glm::translate(baseTransform, cPos + glm::vec3(0.0f, 0.20f * cScale, 0.0f));
            shoot = glm::scale(shoot, glm::vec3(0.12f * cScale, 0.40f * cScale, 0.12f * cScale));
            drawTransformedFrustum(shader, cone, shoot, leafBright, 16.0f, 0.18f);

            // Vibrant tropical flower stalk (Heliconia bracts) on selected plants
            if (c == 1 || c == 3)
            {
                glm::mat4 stem = glm::translate(baseTransform, cPos + glm::vec3(0.0f, 0.30f * cScale, 0.0f));
                stem = glm::scale(stem, glm::vec3(0.03f, 0.60f * cScale, 0.03f));
                drawTransformedCube(shader, cube, stem, leafDark, 12.0f, 0.10f);

                // Alternating flower petal bracts
                for (int b = 0; b < 4; ++b)
                {
                    float by = 0.20f + (float)b * 0.10f;
                    float bSide = (b % 2 == 0) ? 1.0f : -1.0f;
                    glm::mat4 fl = glm::translate(baseTransform, cPos + glm::vec3(bSide * 0.08f * cScale, (0.30f + by) * cScale, 0.0f));
                    fl = glm::rotate(fl, glm::radians(bSide * 35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                    fl = glm::scale(fl, glm::vec3(0.12f * cScale, 0.05f * cScale, 0.04f));
                    drawTransformedCube(shader, cube, fl, (b == 3 ? flowerGold : flowerRed), 20.0f, 0.22f);
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    // 11. TOWERING SHADE TREE (Behind and Beside the Left Staircase)
    // Placed at x=-6.20m, z=-1.80m so it NEVER collides with the stairs!
    // ------------------------------------------------------------------------
    {
        const glm::vec3 treeBase = glm::vec3(-6.20f, 0.0f, -1.80f);

        // Segmented organic leaning trunk rising and curving over the roof
        glm::vec3 curP = treeBase;
        const glm::vec3 trunkSteps[4] = {
            glm::vec3(0.15f, 2.20f, 0.10f),
            glm::vec3(0.40f, 2.30f, 0.25f),
            glm::vec3(0.55f, 2.10f, 0.20f),
            glm::vec3(0.50f, 1.80f, 0.15f)
        };
        const float trunkR[4] = { 0.65f, 0.54f, 0.44f, 0.34f };

        for (int seg = 0; seg < 4; ++seg)
        {
            glm::vec3 nextP = curP + trunkSteps[seg];
            glm::vec3 midP = (curP + nextP) * 0.5f;
            float segLen = glm::length(trunkSteps[seg]);

            glm::mat4 trSeg = glm::translate(baseTransform, midP);
            glm::vec3 dir = glm::normalize(trunkSteps[seg]);
            float yawDeg = glm::degrees(atan2f(dir.x, dir.z));
            float pitchDeg = glm::degrees(asinf(-dir.y));
            trSeg = glm::rotate(trSeg, glm::radians(yawDeg), glm::vec3(0.0f, 1.0f, 0.0f));
            trSeg = glm::rotate(trSeg, glm::radians(pitchDeg + 90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            trSeg = glm::scale(trSeg, glm::vec3(trunkR[seg], segLen, trunkR[seg]));
            drawTransformedFrustum(shader, cylinder, trSeg, treeBark, 16.0f, 0.12f);

            curP = nextP;
        }

        // Lush leafy foliage canopy clusters shading the roof and stairs
        const glm::vec3 canopyOffsets[7] = {
            glm::vec3(-0.4f, 0.0f,  0.5f),
            glm::vec3( 0.6f, 0.5f, -0.4f),
            glm::vec3(-0.8f, 0.8f, -0.2f),
            glm::vec3( 1.1f, 0.2f,  0.6f),
            glm::vec3( 0.0f, 1.3f,  0.0f),
            glm::vec3( 0.8f, 1.1f, -0.5f),
            glm::vec3(-0.5f, 1.2f,  0.7f)
        };
        const float canopySizes[7] = { 2.4f, 2.8f, 2.2f, 2.6f, 3.2f, 2.5f, 2.3f };

        for (int c = 0; c < 7; ++c)
        {
            glm::mat4 can = glm::translate(baseTransform, curP + canopyOffsets[c]);
            can = glm::scale(can, glm::vec3(canopySizes[c], canopySizes[c] * 0.80f, canopySizes[c]));
            glm::vec3 cCol = (c % 2 == 0) ? treeFoliage : glm::vec3(0.20f, 0.48f, 0.16f);
            drawTransformedSphere(shader, sphere, can, cCol, 12.0f, 0.14f);
        }
    }

    // ------------------------------------------------------------------------
    // 12. PEACEFUL SLEEPING DOG (Curled on a Woven Straw Mat near Stairs)
    // Placed safely near the base of the stairs at x=-6.60m, z=2.60m
    // ------------------------------------------------------------------------
    {
        const glm::vec3 dogBase = glm::vec3(-6.60f, 0.02f, 2.60f);

        // Woven straw pet mat
        glm::mat4 mat = glm::translate(baseTransform, dogBase);
        mat = glm::scale(mat, glm::vec3(0.85f, 0.02f, 0.65f));
        drawTransformedCube(shader, cube, mat, matStraw, 12.0f, 0.08f);

        // Curled torso
        glm::mat4 body = glm::translate(baseTransform, dogBase + glm::vec3(0.0f, 0.12f, 0.0f));
        body = glm::rotate(body, glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        body = glm::scale(body, glm::vec3(0.44f, 0.20f, 0.28f));
        drawTransformedSphere(shader, sphere, body, dogFur, 24.0f, 0.20f);

        // Head resting forward on paws
        glm::mat4 head = glm::translate(baseTransform, dogBase + glm::vec3(0.22f, 0.08f, 0.08f));
        head = glm::scale(head, glm::vec3(0.18f, 0.14f, 0.16f));
        drawTransformedSphere(shader, sphere, head, dogFur, 24.0f, 0.20f);

        // Floppy ears
        for (float ez : { -0.06f, 0.06f })
        {
            glm::mat4 ear = glm::translate(baseTransform, dogBase + glm::vec3(0.18f, 0.12f, 0.08f + ez));
            ear = glm::scale(ear, glm::vec3(0.05f, 0.08f, 0.04f));
            drawTransformedSphere(shader, sphere, ear, dogDark, 16.0f, 0.15f);
        }

        // Snout
        glm::mat4 snout = glm::translate(baseTransform, dogBase + glm::vec3(0.30f, 0.06f, 0.09f));
        snout = glm::scale(snout, glm::vec3(0.07f, 0.06f, 0.07f));
        drawTransformedSphere(shader, sphere, snout, dogDark, 32.0f, 0.30f);

        // Front resting paws
        for (float pz : { 0.04f, 0.12f })
        {
            glm::mat4 paw = glm::translate(baseTransform, dogBase + glm::vec3(0.28f, 0.02f, pz));
            paw = glm::scale(paw, glm::vec3(0.12f, 0.04f, 0.05f));
            drawTransformedCube(shader, cube, paw, dogFur, 20.0f, 0.15f);
        }

        // Curled tail
        glm::mat4 tail = glm::translate(baseTransform, dogBase + glm::vec3(-0.22f, 0.06f, -0.06f));
        tail = glm::rotate(tail, glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        tail = glm::scale(tail, glm::vec3(0.16f, 0.05f, 0.05f));
        drawTransformedSphere(shader, sphere, tail, dogFur, 20.0f, 0.15f);
    }
}

#endif // SCENE_OBJECTS_H

// ============================================================================
// SECTION: BoatMotion.h
// ============================================================================
#ifndef BOAT_MOTION_H
#define BOAT_MOTION_H

struct BoatFloatState
{
    float height=0, pitch=0, roll=0;
    float heaveVelocity=0, pitchVelocity=0, rollVelocity=0;
    bool initialized=false;
};

inline void dampedBoatResponse(float& value,float& velocity,float goal,
                               float frequency,float damping,float dt)
{
    // Substeps keep mass/spring response stable across uneven render frames.
    int steps=glm::max(1,(int)ceilf(dt/0.008f));
    float h=dt/steps;
    for(int i=0;i<steps;++i)
    {
        velocity += (frequency*frequency*(goal-value)-2*damping*frequency*velocity)*h;
        value += velocity*h;
    }
}

inline glm::mat4 floatingBoatTransform(BoatFloatState& state,glm::vec3 position,
    float heading,float scale,float time,float dt,float speed=0,float yawRate=0)
{
    float a=glm::radians(heading);
    glm::vec3 forward(sinf(a),0,cosf(a)),right(cosf(a),0,-sinf(a));
    // Fit y = intercept + acrossSlope*x + alongSlope*z to 21 hull samples.
    // Sampling stations, rather than individual peaks at five points, prevents
    // a single crest from sharply rolling the whole boat. These are kinematic
    // support samples, not a full displaced-volume fluid simulation.
    glm::mat3 normalEquations(0);glm::vec3 rhs(0);
    for(int row=0;row<7;++row)
    {
        float z=glm::mix(-1.65f,1.72f,row/6.0f)*scale;
        float t=(z/scale+1.97f)/4.22f;
        float beam=(.73f*(1-powf(t,3.4f))+.018f)*.76f*scale;
        for(int side=-1;side<=1;++side)
        {
            float x=side*beam;glm::vec3 p,n,q=position+forward*z+right*x;
            getWaterSurfaceAt(q.x,q.z,time,p,n);
            glm::vec3 basis(1,x,z);
            normalEquations+=glm::outerProduct(basis,basis);rhs+=basis*p.y;
        }
    }
    glm::vec3 plane=glm::inverse(normalEquations)*rhs;
    float planing=glm::smoothstep(4.0f,9.0f,glm::max(speed,0.0f));
    float height=plane.x-0.055f*scale+0.09f*planing;
    // A positive X rotation lowers local +Z: a rising bow needs negative pitch.
    float pitch=-glm::degrees(atanf(plane.z))-3.0f*planing;
    float roll=glm::degrees(atanf(plane.y));
    roll -= glm::clamp(glm::degrees(atanf(glm::radians(yawRate)*speed/9.81f))*0.40f,-7.0f,7.0f);
    if(!state.initialized)
    {
        state.height=height;state.pitch=pitch;state.roll=roll;state.initialized=true;
    }
    dampedBoatResponse(state.height,state.heaveVelocity,height,8.0f,0.95f,dt);
    dampedBoatResponse(state.pitch,state.pitchVelocity,pitch,6.0f,0.88f,dt);
    dampedBoatResponse(state.roll,state.rollVelocity,roll,5.0f,0.85f,dt);
    glm::mat4 m=glm::translate(glm::mat4(1),glm::vec3(position.x,state.height,position.z));
    m=glm::rotate(m,a,glm::vec3(0,1,0));
    m=glm::rotate(m,glm::radians(state.pitch),glm::vec3(1,0,0));
    m=glm::rotate(m,glm::radians(state.roll),glm::vec3(0,0,1));
    return glm::scale(m,glm::vec3(scale));
}

#endif

// ============================================================================
// SECTION: BeachATV.h
// ============================================================================
#ifndef BEACH_ATV_H
#define BEACH_ATV_H

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

// Closed route computed from the same irregular shoreline as the terrain.
// The seaward sand lane clears the inland loungers and coconut groves while
// leaving dry-ground clearance for all four tyres.
class BeachCircuit
{
    std::vector<glm::vec3> points;
    std::vector<float> distances;
public:
    float length=0;
    BeachCircuit()
    {
        const int count=1024;
        for(int i=0;i<=count;++i)
        {
            float angle=1.570796327f-6.283185307f*i/count;
            glm::vec2 ray(110*cosf(angle),155*sinf(angle));
            float lo=0.0f,hi=1.5f;
            for(int step=0;step<24;++step)
            {
                float radius=(lo+hi)*0.5f;
                float d=getIslandSignedDistance(ray.x*radius,-100+ray.y*radius);
                if(d>6.5f) lo=radius; else hi=radius;
            }
            glm::vec3 p(ray.x*(lo+hi)*0.5f,0,-100+ray.y*(lo+hi)*0.5f);
            p.y=getCoastalTerrainElevation(p.x,p.z);
            if(!points.empty()) length+=glm::distance(p,points.back());
            points.push_back(p);distances.push_back(length);
        }
        points.back()=points.front();
    }
    glm::vec3 sample(float distance,float* heading=nullptr) const
    {
        float s=fmodf(distance,length);if(s<0)s+=length;
        auto upper=std::upper_bound(distances.begin(),distances.end(),s);
        size_t index=glm::min((size_t)(upper-distances.begin()),points.size()-1);
        index=glm::max(index,(size_t)1);
        float t=(s-distances[index-1])/(distances[index]-distances[index-1]);
        glm::vec3 p=glm::mix(points[index-1],points[index],t);
        p.y=getCoastalTerrainElevation(p.x,p.z);
        if(heading)
        {
            glm::vec3 delta=sample(s+.75f)-sample(s-.75f);
            *heading=glm::degrees(atan2f(delta.x,delta.z));
        }
        return p;
    }
    float nearestDistance(const glm::vec3& p) const
    {
        float best=1e30f,result=0;
        for(size_t i=1;i<points.size();++i)
        {
            glm::vec2 a(points[i-1].x,points[i-1].z),b(points[i].x,points[i].z),q(p.x,p.z);
            float t=glm::clamp(glm::dot(q-a,b-a)/glm::dot(b-a,b-a),0.0f,1.0f);
            float d=glm::dot(q-glm::mix(a,b,t),q-glm::mix(a,b,t));
            if(d<best){best=d;result=glm::mix(distances[i-1],distances[i],t);}
        }
        return result;
    }
};

struct SolidObstacle {
    glm::vec2 center,halfSize;
    float yaw=0,radius=0; // radius>0: circle; otherwise oriented box.
    static SolidObstacle circle(glm::vec3 p,float r){return {{p.x,p.z},{0,0},0,r};}
    static SolidObstacle box(const glm::mat4& transform,glm::vec2 half){return {{transform[3].x,transform[3].z},half,atan2f(transform[2].x,transform[2].z),0};}
};
inline std::vector<SolidObstacle> g_atvObstacles;
inline bool atvTouchesObstacle(glm::vec3 p,float heading,const SolidObstacle& obstacle) {
    float angle=glm::radians(heading);
    glm::vec2 right(cosf(angle),-sinf(angle)),forward(sinf(angle),cosf(angle));
    glm::vec2 delta=obstacle.center-glm::vec2(p.x,p.z),half(.97f,1.18f);
    float bound=glm::length(half)+(obstacle.radius>0?obstacle.radius:glm::length(obstacle.halfSize));
    if(glm::dot(delta,delta)>bound*bound)return false;
    if(obstacle.radius>0){
        glm::vec2 local(glm::dot(delta,right),glm::dot(delta,forward));
        glm::vec2 d=local-glm::clamp(local,-half,half);return glm::dot(d,d)<obstacle.radius*obstacle.radius;
    }
    glm::vec2 r(cosf(obstacle.yaw),-sinf(obstacle.yaw)),f(sinf(obstacle.yaw),cosf(obstacle.yaw));
    for(glm::vec2 axis:{right,forward,r,f}){
        float a=half.x*fabsf(glm::dot(right,axis))+half.y*fabsf(glm::dot(forward,axis));
        float b=obstacle.halfSize.x*fabsf(glm::dot(r,axis))+obstacle.halfSize.y*fabsf(glm::dot(f,axis));
        if(fabsf(glm::dot(delta,axis))>=a+b)return false;
    }
    return true;
}
inline bool atvSolidsClear(glm::vec3 p,float heading) {
    for(const auto& obstacle:g_atvObstacles)if(atvTouchesObstacle(p,heading,obstacle))return false;return true;
}
// Sample the sweep at <=2cm translation / <=0.5 degree rotation. The solver
// already runs at 120Hz, so this also covers thin posts at maximum speed.
inline bool atvSweepClear(glm::vec3 from,float fromHeading,glm::vec3 to,float toHeading) {
    float turn=glm::radians(toHeading-fromHeading);
    toHeading=fromHeading+glm::degrees(atan2f(sinf(turn),cosf(turn)));
    int steps=std::max(1,(int)ceilf(std::max(glm::distance(from,to)/.02f,fabsf(toHeading-fromHeading)/.5f)));
    for(int i=1;i<=steps;++i)if(!atvSolidsClear(glm::mix(from,to,(float)i/steps),glm::mix(fromHeading,toHeading,(float)i/steps)))return false;
    return true;
}

struct BeachATVState
{
    glm::vec3 pos=glm::vec3(0,0,43);
    float heading=90,speed=0,steering=0,wheelSpin=0,pitch=0,roll=0;
    float circuitDistance=0,tourTravelled=0,tourStartDistance=0;
    int completedTours=0;
    bool touring=false;
    bool parked=true;
    bool blocked=false;
    float yawRate=0,bodyPitch=0,bodyRoll=0,pitchVelocity=0,rollVelocity=0;
    float reverseWait=0;
};

struct ATVBatteryState {
    float percent=60;bool charging=false,keyDown=false;
    static constexpr float chargePerSecond=100.f/30.f,percentPerMetre=.025f;
    bool hasPower()const{return percent>0;}
    bool press(bool down){bool edge=down && !keyDown;keyDown=down;return edge;}
    void advance(float dt){if(charging && std::isfinite(dt) && dt>0)percent=glm::min(100.f,percent+chargePerSecond*dt);}
    void useDistance(float metres){if(!charging && std::isfinite(metres) && metres>0)percent=glm::max(0.f,percent-percentPerMetre*metres);}
};

inline void startBeachTour(BeachATVState& state,const BeachCircuit& circuit)
{
    state.circuitDistance=circuit.nearestDistance(state.pos);
    state.tourStartDistance=state.circuitDistance;
    state.pos=circuit.sample(state.circuitDistance,&state.heading);
    state.tourTravelled=0;state.speed=0;state.touring=true;state.parked=false;
    state.blocked=false;
}

inline bool atvFootprintIsDry(const glm::vec3& p,float heading)
{
    float a=glm::radians(heading);
    glm::vec3 forward(sinf(a),0,cosf(a)),right(cosf(a),0,-sinf(a));
    for(float along:{-1.15f,1.15f}) for(float across:{-0.94f,0.94f})
    {
        glm::vec3 q=p+forward*along+right*across;
        float d=getIslandSignedDistance(q.x,q.z);
        if(d<3.0f || d>20.0f)return false;
    }
    return true;
}

inline void stepBeachATV(BeachATVState& state,const BeachCircuit& circuit,
                           float throttle,float steer,float dt,bool brake)
{
    if(throttle!=0 || steer!=0 || brake) state.touring=false;
    if(throttle!=0)state.parked=false;
    glm::vec3 oldPosition=state.pos;
    float oldSpeed=state.speed;
    float previousHeading=state.heading;
    state.blocked=false;
    if(state.touring)
    {
        // A gentle six m/s tour, slowing smoothly for the final few metres.
        float remaining=circuit.length-state.tourTravelled;
        float goal=glm::min(6.0f,sqrtf(glm::max(0.0f,remaining)*3.0f));
        float aheadHeading;
        circuit.sample(state.circuitDistance+5,&aheadHeading);
        float bend=glm::radians(aheadHeading-state.heading);
        bend=atan2f(sinf(bend),cosf(bend));
        goal=glm::min(goal,sqrtf(2.0f/glm::max(fabsf(bend)/5.0f,.001f)));
        state.speed+=glm::clamp(goal-state.speed,-3.0f*dt,2.0f*dt);
        float travel=glm::min(remaining,glm::max(state.speed*dt,0.04f*dt));
        state.tourTravelled+=travel;
        state.circuitDistance=state.tourStartDistance+state.tourTravelled;
        state.pos=circuit.sample(state.circuitDistance,&state.heading);
        if(circuit.length-state.tourTravelled<0.04f)
        {
            state.circuitDistance=state.tourStartDistance+circuit.length;
            state.pos=circuit.sample(state.circuitDistance,&state.heading);
            state.tourTravelled=circuit.length;state.touring=false;state.speed=0;
            ++state.completedTours;state.parked=true;
        }
        float change=state.heading-previousHeading;
        while(change>180)change-=360;while(change<-180)change+=360;
        float curvature=glm::radians(change)/glm::max(travel,0.001f);
        state.steering=glm::clamp(glm::degrees(atanf(curvature*1.6f)),-30.0f,30.0f);
    }
    else
    {
        // S brakes first; hold it at rest briefly to select reverse. Space is
        // the independent brake, never an instant velocity reset.
        bool opposing=throttle*state.speed<-.02f;
        if(brake || opposing)
        {
            float deceleration=brake?6.5f:4.5f;
            state.speed-=glm::clamp(state.speed,-deceleration*dt,deceleration*dt);
            state.reverseWait=0;
            if(brake && fabsf(state.speed)<.01f)state.parked=true;
        }
        else
        {
            if(throttle<0 && fabsf(state.speed)<.03f)state.reverseWait+=dt;
            else if(throttle>=0)state.reverseWait=0;
            float drive=(throttle<0 && state.reverseWait<.45f && state.speed>=0)?0:throttle;
            float resistance=.32f+.018f*state.speed*state.speed;
            float acceleration=drive*(drive>0?3.0f:1.8f);
            if(fabsf(state.speed)>.01f)acceleration-=copysignf(resistance,state.speed);
            // Gravity along the slope; modest sand rolling resistance.
            acceleration+=9.81f*sinf(glm::radians(state.pitch));
            if(state.parked && drive==0){acceleration=0;state.speed=0;}
            float next=state.speed+acceleration*dt;
            if(drive==0 && next*state.speed<0)next=0;
            state.speed=glm::clamp(next,-2.0f,9.0f);
        }
        float steeringLimit=32.0f/(1+.018f*state.speed*state.speed);
        float desiredSteering=steer*steeringLimit;
        state.steering+=glm::clamp(desiredSteering-state.steering,-85*dt,85*dt);
        float desiredYaw=state.speed/1.56f*tanf(glm::radians(state.steering));
        desiredYaw=glm::clamp(desiredYaw,-2.8f/glm::max(fabsf(state.speed),1.0f),2.8f/glm::max(fabsf(state.speed),1.0f));
        state.yawRate=glm::mix(state.yawRate,desiredYaw,1-expf(-7*dt));
        float candidateHeading=state.heading+glm::degrees(state.yawRate)*dt;
        float a=glm::radians(candidateHeading);
        glm::vec3 candidate=state.pos+glm::vec3(sinf(a),0,cosf(a))*state.speed*dt;
        if(atvFootprintIsDry(candidate,candidateHeading))
        {state.pos=candidate;state.heading=candidateHeading;}
        else
        {
            // Keep the safe portion of this step instead of visibly bouncing.
            float lo=0,hi=1;
            for(int k=0;k<12;++k)
            {
                float t=(lo+hi)*.5f;
                if(atvFootprintIsDry(glm::mix(state.pos,candidate,t),glm::mix(state.heading,candidateHeading,t)))lo=t;else hi=t;
            }
            state.pos=glm::mix(state.pos,candidate,lo);
            state.heading=glm::mix(state.heading,candidateHeading,lo);
            state.speed*=expf(-18*dt);state.yawRate*=expf(-12*dt);
        }
    }
    // Validate both manual and guided motion. On contact keep the last safe
    // pose and stop longitudinal/yaw velocity; reverse remains available.
    if(!atvSweepClear(oldPosition,previousHeading,state.pos,state.heading)) {
        float turn=glm::radians(state.heading-previousHeading);
        glm::vec3 proposed=state.pos;float proposedHeading=previousHeading+glm::degrees(atan2f(sinf(turn),cosf(turn))),lo=0,hi=1;
        for(int i=0;i<14;++i){float t=(lo+hi)*.5f;
            if(atvSweepClear(oldPosition,previousHeading,glm::mix(oldPosition,proposed,t),glm::mix(previousHeading,proposedHeading,t)))lo=t;else hi=t;}
        state.pos=glm::mix(oldPosition,proposed,lo);state.heading=glm::mix(previousHeading,proposedHeading,lo);
        state.speed=state.yawRate=0;state.blocked=true;state.touring=false;state.parked=true;
    }
    float travelled=glm::length(glm::vec2(state.pos.x-oldPosition.x,state.pos.z-oldPosition.z));
    state.wheelSpin=fmodf(state.wheelSpin+glm::degrees(copysignf(travelled,state.speed)/0.43f),360.0f);
    float a=glm::radians(state.heading);
    glm::vec3 f(sinf(a),0,cosf(a)),r(cosf(a),0,-sinf(a));
    auto height=[&](glm::vec3 p){return getCoastalTerrainElevation(p.x,p.z);};
    float front=(height(state.pos+f*.78f-r*.72f)+height(state.pos+f*.78f+r*.72f))*.5f;
    float rear=(height(state.pos-f*.78f-r*.72f)+height(state.pos-f*.78f+r*.72f))*.5f;
    float left=(height(state.pos+f*.78f-r*.72f)+height(state.pos-f*.78f-r*.72f))*.5f;
    float right=(height(state.pos+f*.78f+r*.72f)+height(state.pos-f*.78f+r*.72f))*.5f;
    state.pos.y=(front+rear)*.5f;
    float acceleration=glm::clamp((state.speed-oldSpeed)/dt,-7.0f,4.0f);
    float turn=state.heading-previousHeading;
    turn=atan2f(sinf(glm::radians(turn)),cosf(glm::radians(turn)));
    float pitchTarget=glm::clamp(acceleration*.55f,-4.0f,3.0f);
    float rollTarget=glm::clamp(state.speed*turn/dt*.8f,-3.0f,3.0f);
    state.pitchVelocity+=(40*(pitchTarget-state.bodyPitch)-11*state.pitchVelocity)*dt;
    state.rollVelocity+=(40*(rollTarget-state.bodyRoll)-11*state.rollVelocity)*dt;
    state.bodyPitch+=state.pitchVelocity*dt;state.bodyRoll+=state.rollVelocity*dt;
    state.pitch=glm::mix(state.pitch,-glm::degrees(atan2f(front-rear,1.56f)),1-expf(-10*dt));
    state.roll=glm::mix(state.roll,glm::degrees(atan2f(right-left,1.44f)),1-expf(-10*dt));
}

inline void updateBeachATV(BeachATVState& state,const BeachCircuit& circuit,
                          float throttle,float steer,float dt,bool brake=false)
{
    // Integrate every elapsed millisecond, including slow rendering frames.
    float remaining=glm::clamp(dt,0.0f,.25f);
    bool blocked=false;
    while(remaining>0.000001f)
    {
        float step=glm::min(remaining,1.0f/120.0f);
        stepBeachATV(state,circuit,throttle,steer,step,brake);remaining-=step;
        blocked|=state.blocked;
    }
    state.blocked=blocked;
}

inline glm::mat4 beachATVTransform(const BeachATVState& state)
{
    glm::mat4 m=glm::translate(glm::mat4(1),state.pos);
    m=glm::rotate(m,glm::radians(state.heading),glm::vec3(0,1,0));
    m=glm::rotate(m,glm::radians(state.pitch),glm::vec3(1,0,0));
    return glm::rotate(m,glm::radians(state.roll),glm::vec3(0,0,1));
}

#endif

// ============================================================================
// SECTION: ATVTracks.h
// ============================================================================
#ifndef ATV_TRACKS_H
#define ATV_TRACKS_H
#include <glm/gtc/type_ptr.hpp>

// Distance-spaced tyre imprints. A bounded GPU ring retains over a full lap
// without thousands of separate draw calls or rebuilding the whole trail.
class ATVTracks
{
    static constexpr size_t capacity=32000, floatsPerStamp=12*8;
    GLuint vao=0,vbo=0;
    size_t cursor=0,count=0;
    bool initialized=false;
    BeachATVState last;
    void imprint(glm::vec3 p,float heading)
    {
        float d=getIslandSignedDistance(p.x,p.z);
        if(d<2.5f || d>21)return;
        float a=glm::radians(heading);
        glm::vec3 f(sinf(a),0,cosf(a)),r(cosf(a),0,-sinf(a));
        std::vector<float> vertices;vertices.reserve(floatsPerStamp);
        auto vertex=[&](glm::vec3 q) {
            q.y=getCoastalTerrainElevation(q.x,q.z)+.035f;
            vertices.insert(vertices.end(),{q.x,q.y,q.z,0,1,0,0,0});
        };
        for(int side:{-1,1})
        {
            glm::vec3 start=p+r*(side*.15f)-f*.045f,end=p+f*.025f;
            glm::vec3 across=glm::normalize(glm::cross(end-start,glm::vec3(0,1,0)))*.025f;
            glm::vec3 v0=start-across,v1=start+across,v2=end-across,v3=end+across;
            vertex(v0);vertex(v2);vertex(v1);vertex(v1);vertex(v2);vertex(v3);
        }
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferSubData(GL_ARRAY_BUFFER,cursor*floatsPerStamp*sizeof(float),vertices.size()*sizeof(float),vertices.data());
        cursor=(cursor+1)%capacity;count=glm::min(count+1,capacity);
    }
public:
    void init()
    {
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);
        glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,capacity*floatsPerStamp*sizeof(float),nullptr,GL_DYNAMIC_DRAW);
        for(int attr=0;attr<3;++attr)
        {
            glVertexAttribPointer(attr,attr==2?2:3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)((attr==0?0:attr==1?3:6)*sizeof(float)));
            glEnableVertexAttribArray(attr);
        }
        glBindVertexArray(0);
    }
    size_t stampCount() const{return count;}
    void update(const BeachATVState& state)
    {
        if(!initialized){last=state;initialized=true;return;}
        float distance=glm::length(glm::vec2(state.pos.x-last.pos.x,state.pos.z-last.pos.z));
        // A route snap/preset is not driving: never draw a line across land.
        if(distance>3){last=state;return;}
        if(distance<.18f)return;
        int steps=(int)floorf(distance/.18f);
        float angle=glm::radians(state.heading-last.heading);
        float turn=glm::degrees(atan2f(sinf(angle),cosf(angle)));
        BeachATVState sample=last;
        for(int i=1;i<=steps;++i)
        {
            float t=i*.18f/distance;
            sample.pos=glm::mix(last.pos,state.pos,t);sample.heading=last.heading+turn*t;
            float a=glm::radians(sample.heading);
            glm::vec3 f(sinf(a),0,cosf(a)),r(cosf(a),0,-sinf(a));
            for(float z:{-.78f,.78f})for(int side:{-1,1})
                imprint(sample.pos+f*z+r*(side*.72f),sample.heading+(z>0?state.steering:0));
        }
        last=sample;
    }
    void draw(Shader& shader) const
    {
        if(!count)return;
        glm::mat4 identity(1);
        glUniformMatrix4fv(shader.location("model"),1,GL_FALSE,glm::value_ptr(identity));
        glUniform3f(shader.location("objectColor"),.39f,.29f,.17f);
        glUniform1f(shader.location("shininess"),4);
        glUniform1f(shader.location("specularStrength"),.015f);
        glUniform1i(shader.location("unlit"),0);
        glUniform1i(shader.location("useTexture"),0);
        glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(-1,-1);
        glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,(GLsizei)(count*12));glBindVertexArray(0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    void cleanup(){glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&vbo);vao=vbo=0;}
};

inline glm::mat4 beachChargingStationTransform(const BeachCircuit& circuit)
{
    float heading;glm::vec3 p=circuit.sample(circuit.length*.12f,&heading);
    glm::vec2 gradient(
        getIslandSignedDistance(p.x+.1f,p.z)-getIslandSignedDistance(p.x-.1f,p.z),
        getIslandSignedDistance(p.x,p.z+.1f)-getIslandSignedDistance(p.x,p.z-.1f));
    gradient=glm::normalize(gradient);
    p+=glm::vec3(gradient.x,0,gradient.y)*5.5f;
    glm::vec3 right(cosf(glm::radians(heading)),0,-sinf(glm::radians(heading)));
    if(glm::dot(right,glm::vec3(gradient.x,0,gradient.y))<0)heading+=180;
    p.y=getCoastalTerrainElevation(p.x,p.z)+.06f;
    return glm::rotate(glm::translate(glm::mat4(1),p),glm::radians(heading),glm::vec3(0,1,0));
}
inline float atvChargerDistance(const BeachATVState& state,const BeachCircuit& circuit) {
    auto point=beachChargingStationTransform(circuit)*glm::vec4(1.23f,0,.65f,1);
    return glm::length(glm::vec2(state.pos.x-point.x,state.pos.z-point.z));
}
inline bool canConnectATVCharger(const BeachATVState& state,const BeachCircuit& circuit) {
    return !state.touring && fabsf(state.speed)<=.1f && atvChargerDistance(state,circuit)<=2.f &&
        atvFootprintIsDry(state.pos,state.heading) && atvSolidsClear(state.pos,state.heading);
}
inline void updatePoweredBeachATV(BeachATVState& state,ATVBatteryState& battery,const BeachCircuit& circuit,
    float throttle,float steer,float dt,bool brake=false) {
    if(battery.charging){state.speed=state.yawRate=0;state.touring=false;state.parked=true;battery.advance(dt);return;}
    if(!battery.hasPower()){throttle=0;state.touring=false;}
    auto before=state.pos;updateBeachATV(state,circuit,throttle,steer,dt,brake);
    battery.useDistance(glm::length(glm::vec2(state.pos.x-before.x,state.pos.z-before.z)));
    if(!battery.hasPower())state.touring=false;
}
#endif

// ============================================================================
// SECTION: Camera.h
// ============================================================================
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
    VIEW_STILT_HOUSE,
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
    // Enabled by the island scene; the separate modeling gallery stays free.
    bool realisticNavigation = false;
    float (*surfaceHeight)(float, float) = nullptr;
    bool riderViewActive = false;
    glm::vec3 riderEye = glm::vec3(0.0f);
    glm::vec3 riderDirection = glm::vec3(0,0,1);

    void constrainToScene()
    {
        if (!realisticNavigation) return;
        projMode = PROJ_PERSPECTIVE;
        auto floorAt = [this](float x,float z) {
            return surfaceHeight ? surfaceHeight(x,z) : 1.15f;
        };
        auto constrainOrbit = [&](glm::vec3& lookAt,float& radius,float azimuth,float& elevation) {
            radius=glm::clamp(radius,2.5f,600.0f);
            elevation=glm::clamp(elevation,3.0f,85.0f);
            lookAt.x=glm::clamp(lookAt.x,-1100.0f,1100.0f);
            lookAt.z=glm::clamp(lookAt.z,-1100.0f,1100.0f);
            float ground=floorAt(lookAt.x,lookAt.z);
            lookAt.y=glm::max(lookAt.y,(ground>1.15f?ground:0.0f)+0.6f);
            float p=glm::radians(elevation),y=glm::radians(azimuth);
            glm::vec3 eye=lookAt+radius*glm::vec3(cosf(p)*sinf(y),sinf(p),cosf(p)*cosf(y));
            // Eye and near plane remain above the maximum swell envelope.
            float lift=glm::max(0.0f,floorAt(eye.x,eye.z)+1.65f-eye.y);
            for(int i=1;i<24;++i)
            {
                glm::vec3 point=glm::mix(lookAt,eye,(float)i/24.0f);
                float terrain=floorAt(point.x,point.z);
                if(terrain>1.15f) lift=glm::max(lift,terrain+0.3f-point.y);
            }
            lookAt.y+=lift;
        };
        // Constrain the destination too, so transitions can finish instead of
        // repeatedly interpolating toward an invalid position inside a hill.
        if(isTransitioning) constrainOrbit(desiredTarget,desiredDistance,desiredYaw,desiredPitch);
        constrainOrbit(target,distance,yaw,pitch);
    }

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
        if (riderViewActive) return riderEye;
        float radPitch = glm::radians(pitch);
        float radYaw = glm::radians(yaw);

        float x = target.x + distance * cosf(radPitch) * sinf(radYaw);
        float y = target.y + distance * sinf(radPitch);
        float z = target.z + distance * cosf(radPitch) * cosf(radYaw);

        return glm::vec3(x, y, z);
    }

    glm::mat4 getViewMatrix() const
    {
        if (riderViewActive)
            return glm::lookAt(riderEye,riderEye+riderDirection,glm::vec3(0,1,0));
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
            return glm::perspective(glm::radians(riderViewActive ? 60.0f : fov), aspectRatio, nearPlane, farPlane);
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
        if (realisticNavigation)
        {
            std::cout << "[Camera] Island navigation uses perspective views above the surface.\n";
            return;
        }
        projMode = (ProjectionMode)((projMode + 1) % PROJ_MODE_COUNT);
        std::cout << "\n>>> [Camera Projection Mode] "
                  << (projMode == PROJ_PERSPECTIVE ? "Perspective Projection (Eye Depth View)" :
                      projMode == PROJ_ORTHO_TOP ? "Orthographic Projection (Top Plan View)" :
                      "Orthographic Projection (Side Elevation View)")
                  << " <<<\n" << std::endl;
    }

    void update(float deltaTime)
    {
        constrainToScene();
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
        constrainToScene();
    }

    void setPreset(ViewPreset preset, GLFWwindow* window = nullptr)
    {
        currentPresetIndex = (int)preset;
        std::string name = "";

        switch (preset)
        {
        case VIEW_BOAT:
            desiredTarget = glm::vec3(8.0f, 1.1f, 78.0f);
            desiredDistance = 8.5f;
            desiredYaw = 20.0f;
            desiredPitch = 14.0f;
            name = "Motorboat [Key: B]";
            break;

        case VIEW_TOWER:
            desiredTarget = glm::vec3(-6.0f, surfaceHeight ? surfaceHeight(-6.0f,-110.0f)+9.0f : 35.0f, -110.0f);
            desiredDistance = 42.0f;
            desiredYaw = 22.0f;
            desiredPitch = 16.0f;
            name = "Hilltop Lighthouse [Key: T]";
            break;

        case VIEW_ROBOT:
            // Focus on the Center Robot lounging under umbrella
            desiredTarget = glm::vec3(0.0f, surfaceHeight ? surfaceHeight(0.0f,39.0f)+1.0f : 1.0f, 39.0f);
            desiredDistance = 8.0f;
            desiredYaw = 38.0f;
            desiredPitch = 18.0f;
            name = "Center Robot & Umbrella [Key: C]";
            break;

        case VIEW_PALM_ROCKS:
            // Focus on the Left Palm Trees and Coastal Rocks
            desiredTarget = glm::vec3(-30.0f, surfaceHeight ? surfaceHeight(-30.0f,29.0f)+3.0f : 3.0f, 29.0f);
            desiredDistance = 16.0f;
            desiredYaw = 42.0f;
            desiredPitch = 16.0f;
            name = "Palm Trees & Rocks [Key: P]";
            break;

        case VIEW_RIGHT_BEACH:
            // Focus on the Right Beach Lounge Setup
            desiredTarget = glm::vec3(28.0f, surfaceHeight ? surfaceHeight(28.0f,36.0f)+1.0f : 1.0f, 36.0f);
            desiredDistance = 16.0f;
            desiredYaw = -32.0f;
            desiredPitch = 16.0f;
            name = "East Shore Loungers [Key: L / 9]";
            break;

        case VIEW_SEA_BOATS:
            // Focus on the Open Sea and Sailing Fleet
            desiredTarget = glm::vec3(0.0f, 1.8f, 115.0f);
            desiredDistance = 42.0f;
            desiredYaw = 10.0f;
            desiredPitch = 12.0f;
            name = "Ocean Sailing Fleet [Key: 7]";
            break;

        case VIEW_STILT_HOUSE:
            desiredTarget = glm::vec3(-24.0f, surfaceHeight ? surfaceHeight(-24.0f, 10.0f) + 2.5f : 3.5f, 10.0f);
            desiredDistance = 16.0f;
            desiredYaw = 35.0f;
            desiredPitch = 14.0f;
            name = "Raised Tropical Stilt House [Key: Y]";
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

// ============================================================================
// SECTION: BeachFurniture.h
// ============================================================================
#ifndef BEACH_FURNITURE_H
#define BEACH_FURNITURE_H
#include <glm/gtc/matrix_transform.hpp>

struct BeachChairPlacement {float x,z,yaw,scale;};

inline std::vector<BeachChairPlacement> safeBeachChairPlacements()
{
    std::vector<BeachChairPlacement> chairs={
        {0,39,0,1}, {-32,34,12,1}, {-24,39,8,1.05f}, {-16,42,5,.96f},
        {-8,44,2,1.02f}, {9,44,-3,1}, {18,41,-8,1.06f}, {28,36,-14,.98f},
        {74,4,-58,1.03f}, {84,-17,-72,.96f}, {91,-39,-84,1.05f},
        {94,-63,-90,1}, {96,-88,-94,1.04f}, {93,-113,-99,.98f}, {86,-137,-108,1.02f}
    };
    for(size_t i=0;i<chairs.size();++i)
    {
        auto& p=chairs[i];
        // Fixed world coordinates are only starting hints. Keep the complete
        // lounger inland of the vehicle lane and highest tide/swash envelope.
        for(int k=0;k<240 && fabsf(getIslandSignedDistance(p.x,p.z)-12.0f)>.15f;++k)
        {
            glm::vec2 gradient(
                getIslandSignedDistance(p.x+.1f,p.z)-getIslandSignedDistance(p.x-.1f,p.z),
                getIslandSignedDistance(p.x,p.z+.1f)-getIslandSignedDistance(p.x,p.z-.1f));
            gradient=glm::normalize(gradient);
            float direction=getIslandSignedDistance(p.x,p.z)<12?1.0f:-1.0f;
            p.x+=gradient.x*.20f*direction;p.z+=gradient.y*.20f*direction;
        }
        if(i>0)
        {
            float gx=getIslandSignedDistance(p.x+.1f,p.z)-getIslandSignedDistance(p.x-.1f,p.z);
            float gz=getIslandSignedDistance(p.x,p.z+.1f)-getIslandSignedDistance(p.x,p.z-.1f);
            p.yaw=glm::degrees(atan2f(-gx,-gz));
        }
    }
    return chairs;
}

inline glm::mat4 beachChairTransform(const BeachChairPlacement& p)
{
    float angle=glm::radians(p.yaw);
    glm::vec3 f(sinf(angle),0,cosf(angle)),r(cosf(angle),0,-sinf(angle)),center(p.x,0,p.z);
    auto height=[&](glm::vec3 q){return getCoastalTerrainElevation(q.x,q.z);};
    float front=height(center+f*.66f*p.scale),rear=height(center-f*.66f*p.scale);
    float left=height(center-r*.52f*p.scale),right=height(center+r*.52f*p.scale);
    center.y=height(center)+.005f;
    glm::mat4 m=glm::rotate(glm::translate(glm::mat4(1),center),angle,glm::vec3(0,1,0));
    m=glm::rotate(m,-atan2f(front-rear,1.32f*p.scale),glm::vec3(1,0,0));
    m=glm::rotate(m,atan2f(right-left,1.04f*p.scale),glm::vec3(0,0,1));
    return glm::scale(m,glm::vec3(p.scale));
}

inline glm::mat4 standingBeachRobotTransform()
{
    static const auto chair=safeBeachChairPlacements()[0];
    glm::vec3 ground(chair.x+3.4f,0,chair.z-2.6f);
    for(int i=0;i<80 && getIslandSignedDistance(ground.x,ground.z)<15.0f;++i)
    {
        glm::vec2 inward(
            getIslandSignedDistance(ground.x+.1f,ground.z)-getIslandSignedDistance(ground.x-.1f,ground.z),
            getIslandSignedDistance(ground.x,ground.z+.1f)-getIslandSignedDistance(ground.x,ground.z-.1f));
        inward=glm::normalize(inward);ground.x+=inward.x*.2f;ground.z+=inward.y*.2f;
    }
    ground.y=getCoastalTerrainElevation(ground.x,ground.z)+.005f;
    float yaw=glm::radians(-12.0f);
    glm::vec3 f(sinf(yaw),0,cosf(yaw)),r(cosf(yaw),0,-sinf(yaw));
    auto height=[&](glm::vec3 q){return getCoastalTerrainElevation(q.x,q.z);};
    glm::mat4 m=glm::rotate(glm::translate(glm::mat4(1),ground),yaw,glm::vec3(0,1,0));
    m=glm::rotate(m,-atan2f(height(ground+f*.3f)-height(ground-f*.3f),.6f),glm::vec3(1,0,0));
    m=glm::rotate(m,atan2f(height(ground+r*.3f)-height(ground-r*.3f),.6f),glm::vec3(0,0,1));
    // Straight legs place the foot soles 0.90 below the pelvis root.
    return glm::translate(m,glm::vec3(0,.90f,0));
}

inline glm::mat4 seatedBeachRobotTransform(const glm::mat4& chair)
{
    // Seat top .4975 + half pelvis .10 + a .005 gap. Knees extend beyond
    // the front slat (.725), so the shins cannot pass through the cushion.
    return glm::translate(chair,glm::vec3(0,.6025f,.36f));
}
inline RobotLegPose seatedBeachRobotLegs()
{
    RobotLegPose pose;pose.shinLength=.5125f;pose.hipDrop=.035f;
    for(int i=0;i<2;++i){pose.hipPitch[i]=-90;pose.kneePitch[i]=90;}
    return pose;
}

struct BeachRobotState {
    glm::vec3 pos;
    float heading=-12,speed=0,phase=0,gaitBlend=0;
    bool active=false,keyDown=false,blocked=false;
    BeachRobotState(){pos=glm::vec3(standingBeachRobotTransform()[3]);pos.y=getCoastalTerrainElevation(pos.x,pos.z);}
    bool press(bool down){bool edge=down&&!keyDown;keyDown=down;if(edge){active=!active;speed=0;blocked=false;}return edge;}
};

inline bool beachRobotPositionClear(glm::vec3 p,const std::vector<SolidObstacle>& obstacles,
    const BeachATVState* atv=nullptr)
{
    // Keep shoulders and both footfalls inside the dry beach corridor.
    float inset=getIslandSignedDistance(p.x,p.z);
    if(inset<5.0f || inset>18.0f)return false;
    for(size_t i=1;i<obstacles.size();++i) // First obstacle is this moving robot.
    {
        const auto& o=obstacles[i];glm::vec2 d=glm::vec2(p.x,p.z)-o.center;
        if(o.radius>0){if(glm::length(d)<o.radius+.46f)return false;}
        else {
            glm::vec2 right(cosf(o.yaw),-sinf(o.yaw)),forward(sinf(o.yaw),cosf(o.yaw));
            glm::vec2 local(glm::dot(d,right),glm::dot(d,forward));
            glm::vec2 nearest=glm::clamp(local,-o.halfSize,o.halfSize);
            if(glm::length(local-nearest)<.46f)return false;
        }
    }
    return !atv || !atvTouchesObstacle(atv->pos,atv->heading,SolidObstacle::circle(p,.46f));
}

inline void updateBeachRobot(BeachRobotState& state,float walk,float turn,float dt,
    const std::vector<SolidObstacle>& obstacles,const BeachATVState* atv=nullptr)
{
    if(!std::isfinite(dt) || dt<=0)return;
    dt=glm::min(dt,.25f);int steps=glm::max(1,(int)ceilf(dt/.008f));float h=dt/steps;
    walk=state.active?glm::clamp(walk,-1.f,1.f):0;turn=state.active?glm::clamp(turn,-1.f,1.f):0;
    state.blocked=false;
    for(int i=0;i<steps;++i)
    {
        state.heading=std::remainder(state.heading+turn*100*h,360.f);
        float goal=walk*(walk<0?1.0f:1.65f);
        state.speed+=glm::clamp(goal-state.speed,-5*h,5*h);
        float yaw=glm::radians(state.heading);glm::vec3 direction(sinf(yaw),0,cosf(yaw));
        glm::vec3 candidate=state.pos+direction*(state.speed*h);
        if(fabsf(state.speed)>.0001f && !beachRobotPositionClear(candidate,obstacles,atv))
        {state.speed=0;state.blocked=true;}
        else {state.phase=std::remainder(state.phase+state.speed*h*6.283185307f/.72f,6.283185307f);state.pos=candidate;}
        if(fabsf(state.speed)<.1f && turn!=0)
            state.phase=std::remainder(state.phase+fabsf(turn)*h*4,6.283185307f);
        float gaitGoal=glm::min(glm::max(fabsf(state.speed)/.6f,fabsf(turn)*.65f),1.f);
        state.gaitBlend=glm::mix(state.gaitBlend,gaitGoal,1-expf(-12*h));
    }
    state.pos.y=getCoastalTerrainElevation(state.pos.x,state.pos.z);
}

struct BeachRobotPose {
    glm::mat4 transform;
    RobotLegPose legs;
    float leftArm=0,rightArm=0;
};
inline BeachRobotPose beachRobotPose(const BeachRobotState& state)
{
    BeachRobotPose pose;
    float pelvisHeight=glm::mix(.88f,.83f,state.gaitBlend)+.008f*state.gaitBlend*cosf(state.phase*2);
    float yaw=glm::radians(state.heading);
    pose.transform=glm::rotate(glm::translate(glm::mat4(1),state.pos+glm::vec3(0,pelvisHeight,0)),yaw,glm::vec3(0,1,0));
    for(int i=0;i<2;++i)
    {
        float phase=fmodf(state.phase+i*3.141592654f+12.566370614f,6.283185307f);
        float u=phase/3.141592654f;
        // Linear stance matches distance travelled; smooth airborne return.
        float stride=u<1?.18f-.36f*u:-.18f+.36f*glm::smoothstep(0.f,1.f,u-1);
        float lift=u<1?0:.12f*sinf((u-1)*3.141592654f);
        float x=i==0?-.16f:.16f,z=.13f+stride*state.gaitBlend;
        glm::vec3 footWorld=glm::vec3(pose.transform*glm::vec4(x,0,z,1));
        float targetY=getCoastalTerrainElevation(footWorld.x,footWorld.z)+.055f+lift*state.gaitBlend;
        float dy=targetY-state.pos.y-pelvisHeight+.07f,dz=z-.13f;
        float reach=glm::clamp(sqrtf(dy*dy+dz*dz),.15f,.7799f);
        float knee=acosf(glm::clamp((reach*reach-2*.39f*.39f)/(2*.39f*.39f),-1.f,1.f));
        float hip=atan2f(-dz,-dy)-knee*.5f;
        pose.legs.hipPitch[i]=glm::degrees(hip);pose.legs.kneePitch[i]=glm::degrees(knee);
        pose.legs.anklePitch[i]=-glm::degrees(hip+knee);
    }
    pose.leftArm=20*sinf(state.phase)*state.gaitBlend;pose.rightArm=-pose.leftArm;
    return pose;
}
#endif

// ============================================================================
// SECTION: IslandNightLights.h
// ============================================================================
#ifndef ISLAND_NIGHT_LIGHTS_H
#define ISLAND_NIGHT_LIGHTS_H
inline float islandNightIntensity(float hour)
{
    if(hour>=18 && hour<20)return glm::smoothstep(18.0f,20.0f,hour);
    if(hour>=20 || hour<5)return 1;
    if(hour<7)return 1-glm::smoothstep(5.0f,7.0f,hour);
    return 0;
}

class IslandNightLights
{
    std::vector<glm::vec4> positions,colors;
public:
    explicit IslandNightLights(const BeachCircuit& circuit)
    {
        for(int i=0;i<18;++i)
        {
            glm::vec3 p=circuit.sample(circuit.length*(i+.45f)/18);
            glm::vec2 inward(
                getIslandSignedDistance(p.x+.1f,p.z)-getIslandSignedDistance(p.x-.1f,p.z),
                getIslandSignedDistance(p.x,p.z+.1f)-getIslandSignedDistance(p.x,p.z-.1f));
            inward=glm::normalize(inward);p+=glm::vec3(inward.x,0,inward.y)*3.6f;
            p.y=getCoastalTerrainElevation(p.x,p.z)+2.65f;
            positions.emplace_back(p,13.0f);colors.emplace_back(1,.72f,.42f,3.0f);
        }
        glm::vec3 bay(beachChargingStationTransform(circuit)*glm::vec4(-.35f,2.70f,0,1));
        positions.emplace_back(bay,10.0f);colors.emplace_back(.74f,.87f,1,3.5f);
    }
    size_t count()const{return positions.size();}
    const std::vector<glm::vec4>& locations()const{return positions;}
    void bind(Shader& shader,float hour) const
    {
        shader.Activate();
        glUniform1i(shader.location("nightLightCount"),(int)positions.size());
        glUniform4fv(shader.location("nightLightPositions[0]"),(int)positions.size(),glm::value_ptr(positions[0]));
        glUniform4fv(shader.location("nightLightColors[0]"),(int)colors.size(),glm::value_ptr(colors[0]));
        glUniform1f(shader.location("nightLightIntensity"),islandNightIntensity(hour));
    }
    void draw(Shader& shader,const CubeMesh& cube,const SphereMesh& sphere,float hour)const
    {
        float night=islandNightIntensity(hour);
        for(size_t i=0;i<positions.size();++i)
        {
            glm::vec3 p(positions[i]);
            auto box=[&](glm::vec3 at,glm::vec3 size,glm::vec3 color,int emissive=0){
                drawTransformedCube(shader,cube,glm::scale(glm::translate(glm::mat4(1),at),size),color,28,.18f,emissive);
            };
            if(i<18)
            {
                box(p-glm::vec3(0,1.35f,0),glm::vec3(.10f,2.6f,.10f),glm::vec3(.12f,.16f,.15f));
                box(p-glm::vec3(0,2.57f,0),glm::vec3(.26f,.16f,.26f),glm::vec3(.32f,.32f,.29f));
                box(p+glm::vec3(0,.16f,0),glm::vec3(.58f,.06f,.40f),glm::vec3(.025f,.07f,.12f));
                box(p+glm::vec3(0,.09f,0),glm::vec3(.32f,.055f,.27f),glm::vec3(.08f,.10f,.09f));
            }
            box(p,glm::vec3(.24f,.075f,.22f),glm::mix(glm::vec3(.45f,.46f,.40f),glm::vec3(colors[i])*1.2f,night),night>.01f?1:0);
        }
    }
};
#endif

// ============================================================================
// SECTION: IslandVegetation.h
// ============================================================================
#ifndef ISLAND_VEGETATION_H
#define ISLAND_VEGETATION_H
// Static coloured mesh batches; fronds and blades sway on the GPU.
class IslandVegetation
{
    GLuint vao=0,vbo=0;
    GLsizei vertexCount=0;
    std::vector<float> data;
    std::vector<glm::vec3> plantRoots;
    int palms=0,understory=0,grass=0,broadleaf=0,ferns=0,flowers=0,fans=0;
    std::vector<glm::vec3> treeRoots;
    void triangle(glm::vec3 a,glm::vec3 b,glm::vec3 c,glm::vec3 color,float wind)
    {
        glm::vec3 cross=glm::cross(b-a,c-a);
        if(glm::length(cross)<.000001f)return;
        glm::vec3 n=glm::normalize(cross);
        for(glm::vec3 p:{a,b,c})data.insert(data.end(),{p.x,p.y,p.z,n.x,n.y,n.z,0,0,color.x,color.y,color.z,wind});
    }
    void leaf(glm::vec3 root,glm::vec3 direction,float length,float width,glm::vec3 color)
    {
        glm::vec3 across=glm::normalize(glm::cross(direction,glm::vec3(0,1,0)));
        auto center=[&](float t){return root+direction*(length*t)+glm::vec3(0,length*(.38f*sinf(t*3.14159265f)-.25f*t*t),0);};
        for(int k=0;k<6;++k)
        {
            float t=k/6.0f,u=(k+1)/6.0f;
            float w0=width*sinf(t*3.14159265f),w1=width*sinf(u*3.14159265f);
            glm::vec3 a=center(t)-across*w0,b=center(t)+across*w0,c=center(u)-across*w1,d=center(u)+across*w1;
            triangle(a,b,c,color,1);triangle(b,d,c,color*.94f,1);
        }
    }
    void branch(glm::vec3 base,glm::vec3 end,float bottom,float top)
    {
        glm::vec3 axis=glm::normalize(end-base),side=glm::normalize(glm::cross(axis,glm::vec3(0,0,1))),across=glm::cross(axis,side);
        for(int j=0;j<8;++j)
        {
            float a=j*6.2831853f/8,b=(j+1)*6.2831853f/8;
            glm::vec3 u=side*cosf(a)+across*sinf(a),v=side*cosf(b)+across*sinf(b);
            triangle(base+u*bottom,base+v*bottom,end+u*top,glm::vec3(.31f,.24f,.16f),0);
            triangle(base+v*bottom,end+v*top,end+u*top,glm::vec3(.35f,.27f,.18f),0);
        }
    }
    void almond(glm::vec3 p,float yaw,float height)
    {
        // Layered, spreading branches and individual broad leaves, not spheres.
        glm::vec3 crown=p+glm::vec3(.4f*cosf(yaw),height,.4f*sinf(yaw));
        branch(p,crown,.24f,.09f);
        for(int tier=0;tier<3;++tier)for(int arm=0;arm<6;++arm)
        {
            float a=yaw+arm*6.2831853f/6+tier*.35f,length=3.1f-tier*.65f;
            glm::vec3 dir(cosf(a),.12f,sinf(a)),base=glm::mix(p,crown,.55f+tier*.19f),end=base+dir*length;
            branch(base,end,.07f,.018f);
            for(int k=1;k<=4;++k)
            {
                glm::vec3 node=base+dir*(length*k/4),side(-dir.z,0,dir.x);
                for(int sign:{-1,1})leaf(node,side*(float)sign+dir*.3f,1.1f,.31f,glm::vec3(.13f+.025f*tier,.34f+.03f*tier,.10f));
                leaf(node,dir,.85f,.27f,glm::vec3(.20f,.42f,.13f));
            }
        }
    }
    void fanPalm(glm::vec3 p,float yaw,float height)
    {
        ++fans;glm::vec3 crown=p+glm::vec3(.35f*cosf(yaw),height,.35f*sinf(yaw));branch(p,crown,.18f,.10f);
        for(int frond=0;frond<9;++frond)
        {
            float a=yaw+frond*6.2831853f/9;
            glm::vec3 dir(cosf(a),.12f,sinf(a)),side(-dir.z,0,dir.x),hub=crown+dir*1.1f;
            branch(crown,hub,.025f,.009f);
            for(int finger=0;finger<15;++finger)
            {
                float t=(finger-7)*.11f;
                glm::vec3 ray=dir*cosf(t)+side*sinf(t);
                leaf(hub,ray,1.45f,.075f,glm::vec3(.18f,.42f,.15f));
            }
        }
    }
public:
    explicit IslandVegetation(const BeachCircuit& circuit)
    {
        glm::mat4 bay=beachChargingStationTransform(circuit);
        for(int i=0;i<5400;++i)
        {
            float x=-119+238*islandHash(i+31,1),z=-254+300*islandHash(i+17,2);
            float shore=getIslandSignedDistance(x,z);
            if(shore<22 || shore>89)continue;
            if(glm::length(glm::vec2(x+6,z+110))<17)continue;
            glm::vec3 p(x,getCoastalTerrainElevation(x,z),z);
            glm::vec3 local(glm::inverse(bay)*glm::vec4(p,1));
            if(fabsf(local.x)<6 && fabsf(local.z)<6)continue;
            // Leave steep rock faces exposed rather than carpeting every slope.
            float slope=glm::length(glm::vec2(getCoastalTerrainElevation(x+.5f,z)-getCoastalTerrainElevation(x-.5f,z),getCoastalTerrainElevation(x,z+.5f)-getCoastalTerrainElevation(x,z-.5f)));
            if(slope>1.1f)continue;
            bool isPalm=i%17==0,isBroadleaf=!isPalm && i%19==0;
            if(isPalm || isBroadleaf)
            {
                bool crowded=false;
                for(glm::vec3 root:treeRoots)if(glm::length(glm::vec2(root.x-x,root.z-z))<4.5f){crowded=true;break;}
                if(crowded)continue;
                treeRoots.push_back(p);
            }
            plantRoots.push_back(p);
            float yaw=islandHash(i,5)*6.2831853f;
            if(i%17==0)
            {
                ++palms;float h=5+islandHash(i,7)*3;
                if(i%3==0){fanPalm(p,yaw,h);continue;}
                glm::vec3 crown=p+glm::vec3(sinf(yaw)*.6f,h,cosf(yaw)*.6f);
                for(int s=0;s<8;++s)
                {
                    float a=s*6.2831853f/8,b=(s+1)*6.2831853f/8;
                    glm::vec3 u(.13f*cosf(a),0,.13f*sinf(a)),v(.13f*cosf(b),0,.13f*sinf(b));
                    triangle(p+u,p+v,crown+u,glm::vec3(.29f,.21f,.13f),0);
                    triangle(p+v,crown+v,crown+u,glm::vec3(.32f,.24f,.15f),0);
                }
                for(int j=0;j<11;++j)
                {
                    float a=yaw+j*6.2831853f/11;
                    glm::vec3 dir(cosf(a),.025f,sinf(a));
                    leaf(crown,dir,3.0f,.045f,glm::vec3(.15f,.40f,.12f));
                    // Feathered leaflets along a curved coconut-palm rachis.
                    for(int k=1;k<9;++k)
                    {
                        float t=k/9.0f;glm::vec3 axis=crown+dir*(3*t)+glm::vec3(0,.45f*sinf(t*3.14f)-1.15f*t*t,0);
                        glm::vec3 side(-dir.z,0,dir.x);
                        for(int sign:{-1,1})leaf(axis,side*(float)sign+dir*.22f,.85f*(1-t)+.35f,.085f,glm::vec3(.10f,.37f,.13f));
                    }
                }
            }
            else if(isBroadleaf)
            {
                ++broadleaf;almond(p,yaw,5.5f+islandHash(i,9)*3.5f);
            }
            else if(i%11==0)
            {
                ++understory;
                bool banana=i%2==0;
                for(int j=0;j<(banana?7:14);++j)
                {
                    float a=yaw+j*6.2831853f/(banana?7:14);
                    leaf(p+glm::vec3(0,banana?1.2f:.55f,0),glm::vec3(cosf(a),.12f,sinf(a)),banana?1.7f:1.1f,banana?.25f:.055f,glm::vec3(.10f,.38f,.12f));
                }
            }
            else if(i%13==0)
            {
                ++ferns;
                for(int j=0;j<9;++j)
                {
                    float a=yaw+j*6.2831853f/9;glm::vec3 dir(cosf(a),.12f,sinf(a)),side(-dir.z,0,dir.x);
                    leaf(p+glm::vec3(0,.15f,0),dir,1.0f,.025f,glm::vec3(.12f,.39f,.15f));
                    for(int k=1;k<7;++k)for(int sign:{-1,1})
                        leaf(p+dir*(k*.12f)+glm::vec3(0,.2f+.2f*sinf(k*.45f),0),side*(float)sign+dir*.15f,.36f-.025f*k,.06f,glm::vec3(.14f,.43f,.16f));
                }
            }
            else if(i%29==0)
            {
                ++flowers;
                for(int stem=0;stem<7;++stem)
                {
                    float a=yaw+stem*2.4f;glm::vec3 dir(cosf(a),.1f,sinf(a)),hub=p+dir*.35f+glm::vec3(0,.75f+.3f*islandHash(i,stem),0);
                    branch(p,hub,.018f,.006f);leaf(hub-glm::vec3(0,.2f,0),dir,.48f,.18f,glm::vec3(.13f,.34f,.09f));
                    glm::vec3 color=stem%2?glm::vec3(.91f,.25f,.31f):glm::vec3(1,.64f,.13f);
                    for(int petal=0;petal<5;++petal)
                    {
                        float t=petal*6.2831853f/5;glm::vec3 ray(cosf(t),0,sinf(t)),side(-ray.z,0,ray.x);
                        triangle(hub,hub+ray*.14f+side*.07f+glm::vec3(0,.04f,0),hub+ray*.20f,color,.5f);
                        triangle(hub,hub+ray*.20f,hub+ray*.14f-side*.07f+glm::vec3(0,.04f,0),color,.5f);
                    }
                }
            }
            else
            {
                ++grass;
                for(int j=0;j<8;++j)
                {
                    float a=yaw+j*2.4f,h=.20f+.30f*islandHash(i,j+20);
                    glm::vec3 r=p+glm::vec3(cosf(a)*.16f,0,sinf(a)*.16f),side(-sinf(a)*.03f,0,cosf(a)*.03f);
                    glm::vec3 tip=r+glm::vec3(cosf(a)*.13f,h,sinf(a)*.13f);
                    triangle(r-side,r+side,tip,glm::vec3(.24f,.38f,.10f)+glm::vec3(.05f*islandHash(i,j)),.6f);
                }
            }
        }
        vertexCount=(GLsizei)data.size()/12;
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(float),data.data(),GL_STATIC_DRAW);
        int counts[]={3,3,2,3,1},offsets[]={0,3,6,8,11};
        for(int a=0;a<5;++a){glVertexAttribPointer(a,counts[a],GL_FLOAT,GL_FALSE,12*sizeof(float),(void*)(offsets[a]*sizeof(float)));glEnableVertexAttribArray(a);}
        glBindVertexArray(0);data.clear();data.shrink_to_fit();
    }
    const std::vector<glm::vec3>& roots()const{return plantRoots;}
    const std::vector<glm::vec3>& woodyRoots()const{return treeRoots;}
    int palmCount()const{return palms;}int understoryCount()const{return understory;}int grassCount()const{return grass;}
    int broadleafCount()const{return broadleaf;}int fernCount()const{return ferns;}int flowerCount()const{return flowers;}int fanPalmCount()const{return fans;}
    void draw(Shader& shader,float time)const
    {
        glm::mat4 identity(1);
        glUniformMatrix4fv(shader.location("model"),1,GL_FALSE,glm::value_ptr(identity));
        glUniform1i(shader.location("foliagePass"),1);
        glUniform1f(shader.location("environmentTime"),time);
        glUniform1i(shader.location("shadingMode"),0);
        glUniform1i(shader.location("useTexture"),0);
        glUniform1i(shader.location("unlit"),0);
        glUniform1f(shader.location("shininess"),8);
        glUniform1f(shader.location("specularStrength"),.035f);
        glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,vertexCount);glBindVertexArray(0);
        glUniform1i(shader.location("foliagePass"),0);
    }
    void cleanup(){glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&vbo);}
};
#endif

// ============================================================================
// SECTION: LighthouseBeam.h
// ============================================================================
#ifndef LIGHTHOUSE_BEAM_H
#define LIGHTHOUSE_BEAM_H
#include <algorithm>
#include <cmath>

struct LighthouseBeamState
{
    float heading=0,speed=55,elevation=0;
    bool paused=false;
    void update(float dt,float aim=0,float speedInput=0,float tilt=0)
    {
        dt=std::max(0.0f,dt);
        speed=std::clamp(speed+speedInput*20*dt,5.0f,90.0f);
        elevation=std::clamp(elevation+tilt*3*dt,-5.0f,5.0f);
        if(aim!=0)paused=true;
        heading=std::fmod(heading+(aim!=0?aim*30:paused?0:speed)*dt,360.0f);
        if(heading<0)heading+=360;
    }
};
#endif

// ============================================================================
// SECTION: LighthouseSite.h
// ============================================================================
#ifndef LIGHTHOUSE_SITE_H
#define LIGHTHOUSE_SITE_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

// A level masonry footing, with skirts buried into the uneven summit.
// Sampling the whole footprint is essential: the centre alone cannot support
// an upright, 4.6 m wide tower on a slope.
class LighthouseSite
{
    GLuint vao=0,vbo=0;
    std::vector<float> vertices;
    float top=0;
    void triangle(glm::vec3 a,glm::vec3 b,glm::vec3 c)
    {
        glm::vec3 n=glm::normalize(glm::cross(b-a,c-a));
        for(glm::vec3 p:{a,b,c})
            vertices.insert(vertices.end(),{p.x,p.y,p.z,n.x,n.y,n.z,p.x,p.z});
    }
public:
    static constexpr float x=-6,z=-110,radius=3.2f;
    LighthouseSite()
    {
        top=getCoastalTerrainElevation(x,z);
        for(int ring=1;ring<=32;++ring)for(int i=0;i<256;++i)
        {
            float a=i*6.283185307f/256,r=radius*ring/32;
            top=std::max(top,getCoastalTerrainElevation(x+r*cosf(a),z+r*sinf(a)));
        }
        top+=.18f;
        for(int i=0;i<128;++i)
        {
            float a=i*6.283185307f/128,b=(i+1)*6.283185307f/128;
            glm::vec3 p(x+radius*cosf(a),top,z+radius*sinf(a));
            glm::vec3 q(x+radius*cosf(b),top,z+radius*sinf(b));
            glm::vec3 lowP(p.x,getCoastalTerrainElevation(p.x,p.z)-1.5f,p.z);
            glm::vec3 lowQ(q.x,getCoastalTerrainElevation(q.x,q.z)-1.5f,q.z);
            triangle(glm::vec3(x,top,z),q,p);
            triangle(p,q,lowP);triangle(q,lowQ,lowP);
        }
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);
        glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STATIC_DRAW);
        for(int i=0;i<3;++i)
        {
            glVertexAttribPointer(i,i==2?2:3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)((i==2?6:i*3)*sizeof(float)));
            glEnableVertexAttribArray(i);
        }
        glBindVertexArray(0);
    }
    float baseHeight()const{return top;}
    glm::mat4 transform()const
    {
        return glm::rotate(glm::translate(glm::mat4(1),glm::vec3(x,top,z)),glm::radians(8.0f),glm::vec3(0,1,0));
    }
    void draw(Shader& shader)const
    {
        glm::mat4 model(1);
        glUniformMatrix4fv(shader.location("model"),1,GL_FALSE,glm::value_ptr(model));
        glUniform3f(shader.location("objectColor"),.38f,.36f,.31f);
        glUniform1i(shader.location("useTexture"),0);
        glUniform1i(shader.location("terrainPalette"),0);
        glUniform1i(shader.location("shadingMode"),0);
        glUniform1i(shader.location("unlit"),0);
        glUniform1f(shader.location("shininess"),10);
        glUniform1f(shader.location("specularStrength"),.06f);
        glBindVertexArray(vao);glDrawArrays(GL_TRIANGLES,0,(GLsizei)vertices.size()/8);glBindVertexArray(0);
    }
    void cleanup(){glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&vbo);}
};
#endif

// Placement-derived collision world. Decorative foliage is intentionally not
// solid; only trunks, poles and actual equipment impede the vehicle.
inline std::vector<SolidObstacle> islandSolidObstacles(const BeachCircuit& circuit,
    const std::vector<BeachChairPlacement>& chairs,const IslandNightLights& lights,
    const std::vector<glm::vec3>& forestRoots,const std::vector<glm::vec3>& vegetationRoots) {
    std::vector<SolidObstacle> result;
    result.push_back(SolidObstacle::circle(glm::vec3(standingBeachRobotTransform()[3]),.44f));
    for(const auto& p:chairs)result.push_back(SolidObstacle::box(beachChairTransform(p),{.64f*p.scale,.85f*p.scale}));
    if(!chairs.empty()){const auto& p=chairs[0];result.push_back(SolidObstacle::circle({p.x-1.6f,0,p.z-1.2f},.15f));}
    auto bay=beachChargingStationTransform(circuit);
    result.push_back(SolidObstacle::box(glm::translate(bay,{1.23f,0,.65f}),{.30f,.26f}));
    for(float z:{-.15f,1.48f})result.push_back(SolidObstacle::circle(glm::vec3(bay*glm::vec4(1.24f,0,z,1)),.08f));
    for(float z:{-1.9f,1.9f})result.push_back(SolidObstacle::circle(glm::vec3(bay*glm::vec4(1.58f,0,z,1)),.09f));
    // The final light is ceiling-mounted above the parking bay, not a pole.
    for(size_t i=0;i+1<lights.locations().size();++i)
        result.push_back(SolidObstacle::circle(glm::vec3(lights.locations()[i]),.12f));
    for(const auto& p:forestRoots)result.push_back(SolidObstacle::circle(p,.30f));
    for(const auto& p:vegetationRoots)result.push_back(SolidObstacle::circle(p,.22f));
    for(glm::vec2 p:std::vector<glm::vec2>{{-30,29},{-15,23},{17,24},{34,28},{-48,24},{48,23},{2,18}})
        result.push_back(SolidObstacle::circle(safeSolidTreeRoot(p.x,p.y),.34f));
    for(const auto& p:islandPandanusPositions())result.push_back(SolidObstacle::circle(safeSolidTreeRoot(p.x,p.z),.30f));
    for(const auto& p:islandBananaPositions())result.push_back(SolidObstacle::circle(safeSolidTreeRoot(p.x,p.z),.22f));
    glm::mat4 house=glm::rotate(glm::translate(glm::mat4(1),glm::vec3(-24,0,10)),glm::radians(15.0f),glm::vec3(0,1,0));
    result.push_back(SolidObstacle::box(house,{4.15f,2.8f}));
    result.push_back(SolidObstacle::box(glm::translate(house,{-5.35f,0,1.4f}),{1.6f,.65f}));
    result.push_back(SolidObstacle::box(glm::translate(house,{5.1f,0,-1.2f}),{1.55f,.60f}));
    return result;
}
inline bool beachCircuitSolidsClear(const BeachCircuit& circuit) {
    for(float d=0;d<circuit.length;d+=.2f){float heading;auto p=circuit.sample(d,&heading);if(!atvSolidsClear(p,heading))return false;}return true;
}

#endif // BEACH_DAY_SCENE_H
