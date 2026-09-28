#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in float WaveElevation;
in float WaterDepth;
in float ShoreDistance;
in float WakeFactor;
in float BowWaveFactor;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 lightDir;       // Direction towards the light source
uniform vec3 viewPos;        // Camera world position
uniform float ambientStrength;
uniform float time;

// Boat interaction
uniform float boatSpeed;

// Textures
uniform sampler2D foamTex;
uniform int useFoamTex;

void main()
{
    vec3 N = normalize(Normal);
    // Actual seas contain short wind waves and capillary ripples, but their
    // contribution at this scale is primarily a subtle change in reflection,
    // not metre-high repeating geometry.
    float rippleX = sin(dot(FragPos.xz, vec2(0.78, 0.31)) * 1.35 + time * 1.9);
    float rippleZ = sin(dot(FragPos.xz, vec2(-0.22, 0.91)) * 1.72 + time * 2.3);
    N = normalize(N + vec3(rippleX * 0.018, 0.0, rippleZ * 0.018));
    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(lightDir);
    vec3 H = normalize(L + V);

    // ========================================================================
    // 1. BATHYMETRIC DEPTH COLOR GRADIENT (Diagram B: Coastal Water Absorption)
    // Shallow crystalline turquoise -> Coastal azure -> Deep offshore navy
    // ========================================================================
    vec3 shallowColor  = vec3(0.12, 0.72, 0.74); // Turquoise / clear tropical lagoon
    vec3 midDepthColor = vec3(0.06, 0.44, 0.68); // Coastal surf / shoaling zone blue
    vec3 deepColor     = vec3(0.02, 0.16, 0.38); // Deep oceanic navy blue

    // Transition based on bathymetric depth
    float depthFactor = clamp(WaterDepth / 3.5, 0.0, 1.0);
    vec3 waterBodyColor = mix(shallowColor, midDepthColor, smoothstep(0.0, 0.4, depthFactor));
    waterBodyColor = mix(waterBodyColor, deepColor, smoothstep(0.4, 1.0, depthFactor));

    // Slight elevation tint: wave crests catch ambient skylight and are slightly brighter
    waterBodyColor += vec3(0.04, 0.08, 0.10) * clamp(WaveElevation * 1.5, -0.2, 0.4);

    // ========================================================================
    // 2. FRESNEL REFLECTION (Schlick's approximation)
    // Grazing angles reflect radiant sky; steep angles reveal water depth
    // ========================================================================
    float NdotV = max(dot(N, V), 0.0);
    float F0 = 0.04; // Reflectance of water at normal incidence (index of refraction ~1.33)
    float fresnel = F0 + (1.0 - F0) * pow(1.0 - NdotV, 4.0);

    // Sky ambient color reflected on water surface
    vec3 skyReflectColor = mix(vec3(0.35, 0.65, 0.90), lightColor, 0.35);

    // ========================================================================
    // 3. SUN SPECULAR GLINT & BLINN-PHONG HIGHLIGHTS
    // Dynamic glistening across angled wave peaks towards the sun
    // ========================================================================
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 160.0);
    vec3 specular = lightColor * spec * 1.45 * (0.25 + 0.75 * fresnel);

    // Diffuse / Ambient lighting
    vec3 ambient = ambientStrength * skyReflectColor * waterBodyColor;
    vec3 diffuse = NdotL * lightColor * waterBodyColor * 0.7;

    vec3 finalColor = ambient + diffuse + mix(vec3(0.0), skyReflectColor, fresnel * 0.65) + specular;

    // ========================================================================
    // 4. WAVE CREST FOAM & SURF BREAKING WHITECAPS (Diagrams A & B)
    // As waves shoal into the surf zone, peaks steepen into white frothy crests.
    // ========================================================================
    // Procedural animated foam lacework coordinates
    vec2 foamUV = FragPos.xz * 0.28 + vec2(time * 0.04, time * 0.02);
    float foamNoise = 0.5 + 0.5 * sin(foamUV.x * 6.28 + sin(foamUV.y * 5.0 + time * 1.5));

    if (useFoamTex == 1)
    {
        foamNoise *= texture(foamTex, foamUV).r;
    }

    // Calm offshore swell does not foam continuously. Breaking is restricted
    // to the depth-limited nearshore zone, where the seabed steepens crests.
    float breakingZone = 1.0 - smoothstep(13.0, 26.0, ShoreDistance);
    float surfaceSlope = 1.0 - clamp(N.y, 0.0, 1.0);
    float crestBand = smoothstep(0.30, 0.55, WaveElevation);
    float crestFoam = crestBand * smoothstep(0.010, 0.055, surfaceSlope)
                    * smoothstep(0.52, 0.82, foamNoise) * breakingZone;

    // Shoreline Swash / Backwash Foam (Diagram A: Swash/Backwash zone at Z < 3.5)
    float shoreDist = ShoreDistance;
    float swashPhase = sin(time * 1.8 - shoreDist * 2.2 + FragPos.x * 0.08);
    float shoreFoam = (1.0 - smoothstep(0.35, 3.8, shoreDist))
                    * smoothstep(0.2, 0.8, swashPhase) * (0.7 + 0.3 * foamNoise);

    // ========================================================================
    // 5. BOAT WAKE FOAM & BOW SPRAY
    // Dynamic foam in the boat's wake trail and bow wave area
    // ========================================================================
    float wakeFoam = 0.0;
    if (boatSpeed > 0.2)
    {
        // Wake trail: churning white water behind the boat
        float wakeNoise = 0.5 + 0.5 * sin(FragPos.x * 4.0 + time * 3.5) *
                          cos(FragPos.z * 3.0 - time * 2.8);
        wakeFoam = WakeFactor * (0.65 + 0.35 * wakeNoise) * clamp(boatSpeed / 5.0, 0.0, 1.0);

        // Bow spray: bright white water pushed aside by the hull
        float bowNoise = 0.5 + 0.5 * sin(FragPos.x * 8.0 + time * 6.0);
        wakeFoam += BowWaveFactor * (0.8 + 0.2 * bowNoise) * clamp(boatSpeed / 4.0, 0.0, 1.0);
    }

    float totalFoam = clamp(crestFoam * 0.82 + shoreFoam * 1.05 + wakeFoam * 1.25, 0.0, 1.0);
    vec3 foamColor = vec3(0.96, 0.98, 1.0); // Pure frothy white seafoam

    // Wake foam has a slight blue-white tint (aerated sea water)
    vec3 wakeFoamTint = mix(foamColor, vec3(0.82, 0.92, 1.0), 0.3);
    vec3 blendedFoamColor = mix(foamColor, wakeFoamTint, clamp(wakeFoam / (totalFoam + 0.001), 0.0, 1.0));

    finalColor = mix(finalColor, blendedFoamColor, totalFoam * 0.88);

    // ========================================================================
    // 6. ALPHA TRANSPARENCY
    // Shallow water near the shore is translucent to show golden sand beneath
    // Wave foam and deeper offshore water are progressively more opaque
    // ========================================================================
    float alpha = mix(0.72, 0.94, clamp(WaterDepth / 1.5, 0.0, 1.0));
    alpha = max(alpha, totalFoam * 0.95);

    FragColor = vec4(finalColor, alpha);
}
