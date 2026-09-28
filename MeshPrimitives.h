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
        float beachToGrass = glm::smoothstep(13.0f, 21.0f, shoreDistance);
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
        return 0.08f + beachToGrass * 0.72f + hillMass + summits + erosion;
    }

    float offshore = -shoreDistance;
    float shallowShelf = -0.15f - 0.12f * offshore;
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
