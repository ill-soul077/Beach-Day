#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 GouraudColor;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDir;       // Direction towards the light source
uniform vec3 viewPos;        // Camera world position
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform float objectAlpha;     // Used by translucent emissive effects such as the beacon beam
uniform int beaconBeamPass;
uniform int unlit;           // 1 for self-illuminated / shadow objects, 0 for lit
uniform int shadingMode;     // 0 = Phong (per-fragment), 1 = Gouraud (per-vertex), 2 = Flat (faceted)

// Texture Mapping
uniform sampler2D textureMap;
uniform int useTexture;      // 0 = off, 1 = on
uniform float texScale;      // UV tiling scale
uniform int terrainPalette;  // 1 = Saint Martin coral-soil/vegetation palette

// Ground Shadow Pass
uniform int isShadowPass;    // 1 = render flattened dark projected shadow
uniform vec3 shadowColor;

float islandSignedDistance(vec2 p)
{
    vec2 n = vec2(p.x / 110.0, (p.y + 100.0) / 155.0);
    float angle = atan(n.y, n.x);
    float outline = 1.0
        + 0.110 * sin(angle * 3.0 + 0.4)
        + 0.065 * sin(angle * 5.0 - 0.8)
        + 0.045 * cos(angle * 7.0 + 1.1)
        + 0.085 * max(cos(angle - 0.10), 0.0)
        - 0.075 * pow(max(cos(angle + 1.0), 0.0), 4.0);
    return (outline - length(n)) * 100.0;
}

void main()
{
    // Fast path: Ground projected shadow rendering
    if (isShadowPass == 1)
    {
        FragColor = vec4(shadowColor, 1.0);
        return;
    }

    // Apply optional texture detail to base object color
    vec3 baseColor = objectColor;
    if (terrainPalette == 1)
    {
        float broadNoise = sin(FragPos.x * 0.19 + FragPos.z * 0.11) * 0.5
                         + sin(FragPos.x * 0.47 - FragPos.z * 0.29) * 0.25;
        float steepness = 1.0 - abs(normalize(Normal).y);
        float coastDistance = islandSignedDistance(FragPos.xz);
        float vegetation = smoothstep(13.0, 22.0, coastDistance + broadNoise);
        vec3 coralSand = vec3(0.58, 0.48, 0.34);
        vec3 dryGrass  = vec3(0.25, 0.35, 0.12);
        vec3 jungle    = vec3(0.055, 0.25, 0.085);
        vec3 coralRock = vec3(0.31, 0.29, 0.25);
        vec3 inlandColor = mix(dryGrass, jungle, smoothstep(0.3, 0.9, vegetation + broadNoise * 0.16));
        baseColor = mix(coralSand, inlandColor, vegetation);
        baseColor = mix(baseColor, coralRock, smoothstep(0.24, 0.58, steepness));
    }
    if (useTexture == 1)
    {
        vec4 texSample = texture(textureMap, TexCoord * texScale);
        baseColor = baseColor * texSample.rgb * 1.35; // Blend texture detail
    }

    if (unlit == 1)
    {
        float alpha = objectAlpha;
        if (beaconBeamPass == 1)
        {
            // Frustum V runs from the distant open end (0) to the optic (1).
            // Air scattering is strongest near the source and fades smoothly
            // with distance rather than ending in a bright geometric disc.
            float distanceFade = mix(0.08, 1.0, pow(clamp(TexCoord.y, 0.0, 1.0), 0.72));
            float hazeVariation = 0.90 + 0.10 * sin(FragPos.x * 0.17 + FragPos.z * 0.13);
            alpha *= distanceFade * hazeVariation;
        }
        FragColor = vec4(baseColor, alpha);
        return;
    }

    // ========================================================================
    // 1. GOURAUD SHADING (per-vertex color interpolation)
    // Applied to smooth, expansive curved surfaces like Sea & Sand terrain
    // ========================================================================
    if (shadingMode == 1)
    {
        if (useTexture == 1)
        {
            vec4 texSample = texture(textureMap, TexCoord * texScale);
            FragColor = vec4(GouraudColor * texSample.rgb * 1.35, objectAlpha);
        }
        else
        {
            FragColor = vec4(GouraudColor, objectAlpha);
        }
        return;
    }

    // ========================================================================
    // 2. FLAT SHADING vs. 3. PHONG SHADING
    // Flat: faceted normal computed from screen derivatives (rocks / distant)
    // Phong: smooth interpolated surface normal (robot, umbrella, boat, palms)
    // ========================================================================
    vec3 norm;
    if (shadingMode == 2)
    {
        // Compute face normal dynamically via screen-space derivatives
        vec3 fdx = dFdx(FragPos);
        vec3 fdy = dFdy(FragPos);
        norm = normalize(cross(fdx, fdy));
    }
    else
    {
        norm = normalize(Normal);
    }

    // 1. Ambient Lighting
    vec3 ambient = ambientStrength * lightColor;

    // 2. Diffuse Lighting
    vec3 lightDirection = normalize(lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Specular Lighting (Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 1.0));
    vec3 specular = specularStrength * spec * lightColor;

    // Combine lighting components with base surface color
    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, objectAlpha);
}
