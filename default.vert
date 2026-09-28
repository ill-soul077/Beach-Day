#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 GouraudColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

// Lighting parameters for Gouraud Shading (per-vertex)
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDir;       // Direction towards light source
uniform vec3 viewPos;        // Camera world position
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform int shadingMode;     // 0 = Phong, 1 = Gouraud, 2 = Flat
uniform int unlit;
uniform int isShadowPass;

void main()
{
    // Fast path for ground projected shadow pass (avoids singular matrix inversion)
    if (isShadowPass == 1)
    {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = vec3(0.0, 1.0, 0.0);
        TexCoord = aTexCoord;
        GouraudColor = vec3(0.0);
        gl_Position = proj * view * vec4(FragPos, 1.0);
        return;
    }

    // Compute world-space position
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Normal matrix to prevent non-uniform scale distortion
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal);
    
    TexCoord = aTexCoord;

    // GOURAUD SHADING: Compute lighting equation directly at vertex level
    if (shadingMode == 1 && unlit == 0)
    {
        vec3 ambient = ambientStrength * lightColor;

        vec3 norm = normalize(Normal);
        vec3 lightDirection = normalize(lightDir);
        float diff = max(dot(norm, lightDirection), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDirection, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 1.0));
        vec3 specular = specularStrength * spec * lightColor;

        GouraudColor = (ambient + diffuse + specular) * objectColor;
    }
    else
    {
        GouraudColor = objectColor;
    }
    
    gl_Position = proj * view * vec4(FragPos, 1.0);
}
