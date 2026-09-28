#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <vector>
#include <cmath>
#include <cstdlib>
#include <glad/glad.h>

// ============================================================================
// TEXTURE MANAGER (PROCEDURAL TEXTURE GENERATION FOR SCENE OBJECTS)
// Fulfills Proposal Requirement:
// "Basic texture mapping on sand, water, boat sail, and palm leaves"
// Procedurally synthesizes high-quality textures in memory with mipmapping.
// ============================================================================
enum TextureType
{
    TEX_NONE = 0,
    TEX_SAND,
    TEX_WATER,
    TEX_SAIL,
    TEX_PALM,
    TEX_FOAM,
    TEX_COUNT
};

class TextureManager
{
public:
    GLuint textures[TEX_COUNT];
    bool initialized;

    TextureManager() : initialized(false)
    {
        for (int i = 0; i < TEX_COUNT; ++i)
            textures[i] = 0;
    }

    void init()
    {
        if (initialized) return;

        textures[TEX_SAND]  = createSandTexture(128, 128);
        textures[TEX_WATER] = createWaterTexture(128, 128);
        textures[TEX_SAIL]  = createSailTexture(128, 128);
        textures[TEX_PALM]  = createPalmTexture(128, 128);
        textures[TEX_FOAM]  = createFoamTexture(128, 128);

        initialized = true;
    }

    void bind(TextureType type, GLenum textureUnit = GL_TEXTURE0) const
    {
        glActiveTexture(textureUnit);
        if (type > TEX_NONE && type < TEX_COUNT)
        {
            glBindTexture(GL_TEXTURE_2D, textures[type]);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void unbind(GLenum textureUnit = GL_TEXTURE0) const
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void cleanup()
    {
        for (int i = 1; i < TEX_COUNT; ++i)
        {
            if (textures[i])
            {
                glDeleteTextures(1, &textures[i]);
                textures[i] = 0;
            }
        }
        initialized = false;
    }

private:
    GLuint uploadTexture(int width, int height, const std::vector<unsigned char>& data)
    {
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        return texId;
    }

    // 1. SAND TEXTURE: Natural coastal quartz & silica sand with delicate wind ripples and mineral grains
    GLuint createSandTexture(int width, int height)
    {
        std::vector<unsigned char> data(width * height * 4);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int idx = (y * width + x) * 4;

                // Subtle wind-blown sand drift ripples
                float ripple = sinf((float)y * 0.28f + sinf((float)x * 0.12f) * 1.8f) * 6.5f;

                // Fine crystalline quartz & feldspar grain variation
                float grain = (float)(rand() % 21) - 10.0f;

                // Natural coastal sand tone: soft warm ecru / champagne quartz
                int r = (int)glm::clamp(216.0f + ripple + grain, 0.0f, 255.0f);
                int g = (int)glm::clamp(202.0f + ripple * 0.92f + grain * 0.90f, 0.0f, 255.0f);
                int b = (int)glm::clamp(176.0f + ripple * 0.80f + grain * 0.75f, 0.0f, 255.0f);

                // Natural dark mineral flecks (hornblende / biotite mica)
                if ((rand() % 100) < 4)
                {
                    r = 135 + (rand() % 25);
                    g = 120 + (rand() % 25);
                    b = 95  + (rand() % 20);
                }

                data[idx + 0] = (unsigned char)r;
                data[idx + 1] = (unsigned char)g;
                data[idx + 2] = (unsigned char)b;
                data[idx + 3] = 255;
            }
        }
        return uploadTexture(width, height, data);
    }

    // 2. WATER TEXTURE: Ocean caustics wave pattern with shimmering ripple crests
    GLuint createWaterTexture(int width, int height)
    {
        std::vector<unsigned char> data(width * height * 4);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int idx = (y * width + x) * 4;

                // Overlapping caustic waves
                float wave1 = sinf((float)x * 0.22f + (float)y * 0.18f);
                float wave2 = sinf((float)x * 0.15f - (float)y * 0.25f);
                float wave3 = cosf((float)x * 0.30f + sinf((float)y * 0.20f));
                float caustics = (wave1 + wave2 + wave3) / 3.0f; // -1.0 to 1.0

                // Foam / sparkling crest
                float crest = (caustics > 0.45f) ? (caustics - 0.45f) * 120.0f : 0.0f;

                int r = (int)glm::clamp(30.0f  + caustics * 25.0f + crest * 1.5f, 0.0f, 255.0f);
                int g = (int)glm::clamp(145.0f + caustics * 45.0f + crest * 1.8f, 0.0f, 255.0f);
                int b = (int)glm::clamp(215.0f + caustics * 35.0f + crest * 1.5f, 0.0f, 255.0f);

                data[idx + 0] = (unsigned char)r;
                data[idx + 1] = (unsigned char)g;
                data[idx + 2] = (unsigned char)b;
                data[idx + 3] = 255;
            }
        }
        return uploadTexture(width, height, data);
    }

    // 3. SAIL TEXTURE: Crisp white canvas weave with vibrant yacht racing stripes
    GLuint createSailTexture(int width, int height)
    {
        std::vector<unsigned char> data(width * height * 4);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int idx = (y * width + x) * 4;

                // Subtle fabric weave cross-hatching
                int weave = ((x % 2 == 0) ^ (y % 2 == 0)) ? 8 : -8;

                // Bold nautical racing stripe across the middle (diagonal)
                float u = (float)x / (float)width;
                float v = (float)y / (float)height;
                float stripeCoord = u * 0.8f + v * 0.4f;

                int r, g, b;
                if (stripeCoord >= 0.45f && stripeCoord <= 0.55f)
                {
                    // Bold nautical red stripe
                    r = 215 + weave;
                    g = 35 + weave;
                    b = 35 + weave;
                }
                else if (stripeCoord >= 0.58f && stripeCoord <= 0.65f)
                {
                    // Royal navy stripe
                    r = 25 + weave;
                    g = 45 + weave;
                    b = 110 + weave;
                }
                else
                {
                    // Clean canvas off-white
                    r = 245 + weave;
                    g = 245 + weave;
                    b = 240 + weave;
                }

                data[idx + 0] = (unsigned char)glm::clamp(r, 0, 255);
                data[idx + 1] = (unsigned char)glm::clamp(g, 0, 255);
                data[idx + 2] = (unsigned char)glm::clamp(b, 0, 255);
                data[idx + 3] = 255;
            }
        }
        return uploadTexture(width, height, data);
    }

    // 4. PALM LEAF TEXTURE: Frond leaflet striations with central green rachis vein
    GLuint createPalmTexture(int width, int height)
    {
        std::vector<unsigned char> data(width * height * 4);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int idx = (y * width + x) * 4;

                // Central spine running along center
                float distFromCenter = fabsf((float)x - (float)width * 0.5f) / ((float)width * 0.5f);

                // Leaflet vein striations
                float veins = sinf((float)y * 0.85f + distFromCenter * 8.0f) * 18.0f;

                int r, g, b;
                if (distFromCenter < 0.08f)
                {
                    // Central yellow-green rachis stem
                    r = 110 + (int)veins;
                    g = 185 + (int)veins;
                    b = 40;
                }
                else
                {
                    // Lush tropical green blade with darker margin
                    r = (int)glm::clamp(45.0f + veins * 0.5f - distFromCenter * 20.0f, 0.0f, 255.0f);
                    g = (int)glm::clamp(165.0f + veins - distFromCenter * 30.0f, 0.0f, 255.0f);
                    b = (int)glm::clamp(40.0f + veins * 0.4f, 0.0f, 255.0f);
                }

                data[idx + 0] = (unsigned char)glm::clamp(r, 0, 255);
                data[idx + 1] = (unsigned char)glm::clamp(g, 0, 255);
                data[idx + 2] = (unsigned char)glm::clamp(b, 0, 255);
                data[idx + 3] = 255;
            }
        }
        return uploadTexture(width, height, data);
    }

    // 5. FOAM TEXTURE: Cellular frothy seafoam pattern for wave crests and shoreline swash
    GLuint createFoamTexture(int width, int height)
    {
        std::vector<unsigned char> data(width * height * 4);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int idx = (y * width + x) * 4;

                // Multi-scale cellular noise for seafoam lacework bubbles
                float fx = (float)x / (float)width * 8.0f;
                float fy = (float)y / (float)height * 8.0f;

                float bubble1 = sinf(fx * 3.14159f) * cosf(fy * 3.14159f);
                float bubble2 = sinf((fx + fy) * 2.5f) * cosf((fx - fy) * 2.5f);
                float bubble3 = sinf(fx * 6.28f + sinf(fy * 4.0f));

                float val = (bubble1 + bubble2 * 0.7f + bubble3 * 0.5f) / 2.2f;
                val = fabsf(val); // Sharp cellular ridge edges
                val = 1.0f - val;  // Invert so cell walls/bubbles are bright
                val = powf(val, 2.2f); // High contrast frothy bubbles

                int intensity = (int)glm::clamp(val * 255.0f, 0.0f, 255.0f);

                data[idx + 0] = (unsigned char)intensity;
                data[idx + 1] = (unsigned char)intensity;
                data[idx + 2] = (unsigned char)intensity;
                data[idx + 3] = (unsigned char)intensity;
            }
        }
        return uploadTexture(width, height, data);
    }
};

#endif // TEXTURE_MANAGER_H
