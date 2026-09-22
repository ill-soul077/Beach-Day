#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDir;       // Direction towards the light source
uniform vec3 viewPos;        // Camera world position
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform int unlit;           // 1 for self-illuminated objects (like the Sun), 0 for lit

void main()
{
    if (unlit == 1)
    {
        FragColor = vec4(objectColor, 1.0);
        return;
    }

    // 1. Ambient Lighting
    vec3 ambient = ambientStrength * lightColor;

    // 2. Diffuse Lighting
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Specular Lighting (Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine Phong components with object color
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
