#ifndef BEACH_DAY_RENDERING_H
#define BEACH_DAY_RENDERING_H

#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_map>
#include <glm/glm.hpp>

// Embedded shaders: unchanged GLSL sources, separated into labelled constants.
namespace BeachDayShaders {
inline constexpr char DefaultVertex[] = R"BEACH_GLSL(#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aPlantColor;
layout (location = 4) in float aPlantWind;
layout (location = 5) in mat4 instanceModel;
layout (location = 9) in vec3 instanceColor;
layout (location = 10) in vec3 instanceMaterial;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 GouraudColor;
out vec3 GouraudSpecular;
out vec3 PlantColor;
out float ObjectHeight;

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
uniform int shadowDepthPass;
uniform int forceShadingMode;
uniform int terrainPalette;
uniform float seaLevel;
uniform int foliagePass;
uniform float environmentTime;
uniform int instancedPass;
out vec3 InstanceColor;
out vec3 InstanceMaterial;

void main()
{
    ObjectHeight=aPos.y;
    mat4 effectiveModel=instancedPass==1?instanceModel:model;
    InstanceColor=instancedPass==1?instanceColor:objectColor;
    InstanceMaterial=instancedPass==1?instanceMaterial:vec3(shininess,specularStrength,float(unlit));
    vec3 world=vec3(effectiveModel*vec4(aPos,1));
    if(foliagePass==1)world.x+=sin(environmentTime*1.4+world.x*.22+world.z*.13)*aPlantWind*.09;
    PlantColor=aPlantColor;
    // Fast path for ground projected shadow pass (avoids singular matrix inversion)
    if (isShadowPass == 1 || shadowDepthPass == 1)
    {
        FragPos = world;
        Normal = vec3(0.0, 1.0, 0.0);
        TexCoord = aTexCoord;
        GouraudColor = vec3(0.0);
        GouraudSpecular = vec3(0.0);
        gl_Position = proj * view * vec4(FragPos, 1.0);
        return;
    }

    // Compute world-space position
    FragPos = world;
    
    // Normal matrix to prevent non-uniform scale distortion
    mat3 normalMatrix = transpose(inverse(mat3(effectiveModel)));
    Normal = normalize(normalMatrix * aNormal);
    
    TexCoord = aTexCoord;

    // GOURAUD SHADING: Compute lighting equation directly at vertex level
    int mode=forceShadingMode==0?shadingMode:(forceShadingMode==1?2:forceShadingMode==2?1:0);
    GouraudSpecular=vec3(0);
    if (mode == 1 && InstanceMaterial.z == 0)
    {
        vec3 ambient = ambientStrength * lightColor;

        vec3 norm = normalize(Normal);
        vec3 lightDirection = normalize(lightDir);
        float diff = max(dot(norm, lightDirection), 0.0);
        if(foliagePass==1)diff=max(diff,max(dot(-norm,lightDirection),0)*.35);
        vec3 diffuse = diff * lightColor;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDirection, norm);
        float wet=terrainPalette==1?1-smoothstep(seaLevel+.06,seaLevel+.55,FragPos.y):0;
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), mix(max(InstanceMaterial.x, 1.0),72.0,wet));
        vec3 specular = mix(InstanceMaterial.y,.28,wet) * spec * lightColor;

        GouraudColor = ambient + diffuse;
        GouraudSpecular = specular;
    }
    else
    {
        GouraudColor = InstanceColor;
    }
    
    gl_Position = proj * view * vec4(FragPos, 1.0);
}
)BEACH_GLSL";
inline constexpr char DefaultFragment[] = R"BEACH_GLSL(#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 GouraudColor;
in vec3 GouraudSpecular;
in vec3 PlantColor;
in float ObjectHeight;
in vec3 InstanceColor;
in vec3 InstanceMaterial;

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
uniform int lighthouseShaftPass;
uniform int unlit;           // 1 for self-illuminated / shadow objects, 0 for lit
uniform int shadingMode;     // 0 = Phong (per-fragment), 1 = Gouraud (per-vertex), 2 = Flat (faceted)

// Texture Mapping
uniform sampler2D textureMap;
uniform int useTexture;      // 0 = off, 1 = on
uniform float texScale;      // UV tiling scale
uniform int terrainPalette;  // 1 = Saint Martin coral-soil/vegetation palette
uniform int boatHullPass;
uniform int boatSailPass;
uniform float boatWaterHeight;

// Ground Shadow Pass
uniform int isShadowPass;    // 1 = render flattened dark projected shadow
uniform vec3 shadowColor;
uniform int shadowDepthPass;
uniform int forceShadingMode;
uniform float seaLevel;
uniform sampler2D sunShadowMap;
uniform mat4 sunShadowMatrix;
uniform int sunShadowsEnabled;
uniform int foliagePass;
uniform int nightLightCount;
uniform vec4 nightLightPositions[20];
uniform vec4 nightLightColors[20];
uniform float nightLightIntensity;

vec3 nightLighting(vec3 base,vec3 normal)
{
    if(nightLightIntensity<=0)return vec3(0);
    vec3 result=vec3(0),n=normalize(normal),v=normalize(viewPos-FragPos);
    for(int i=0;i<20;++i)
    {
        if(i>=nightLightCount)break;
        vec3 delta=nightLightPositions[i].xyz-FragPos;
        float distance=length(delta),radius=nightLightPositions[i].w;
        if(distance>=radius)continue;
        vec3 l=delta/max(distance,.01);
        float down=smoothstep(-.15,.35,l.y);
        float attenuation=(1-smoothstep(radius*.65,radius,distance))/(1+.12*distance+.055*distance*distance);
        float diffuse=max(dot(n,l),0);
        if(foliagePass==1)diffuse=max(diffuse,max(dot(-n,l),0)*.45);
        float spec=pow(max(dot(n,normalize(l+v)),0),max(shininess,16.0))*specularStrength;
        result+=nightLightColors[i].rgb*nightLightColors[i].w*attenuation*down*(base*diffuse+spec);
    }
    return result*nightLightIntensity;
}

float sunVisibility(vec3 position,vec3 normal)
{
    if(sunShadowsEnabled==0)return 1.0;
    vec4 projected=sunShadowMatrix*vec4(position,1);
    vec3 p=projected.xyz/projected.w*.5+.5;
    if(p.z<=0 || p.z>=1 || any(lessThan(p.xy,vec2(0))) || any(greaterThan(p.xy,vec2(1))))return 1.0;
    float bias=max(.00009,.00020*(1-max(dot(normalize(normal),normalize(lightDir)),0)));
    vec2 texel=1.0/vec2(textureSize(sunShadowMap,0));float visible=0;
    for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y)
        visible+=p.z-bias<=texture(sunShadowMap,p.xy+vec2(x,y)*texel).r?1.0:0.0;
    return visible/9.0;
}

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
    if(shadowDepthPass==1){FragColor=vec4(1);return;}
    // Fast path: Ground projected shadow rendering
    if (isShadowPass == 1)
    {
        FragColor = vec4(shadowColor, 1.0);
        return;
    }

    // Apply optional texture detail to base object color
    vec3 baseColor = foliagePass==1?PlantColor:InstanceColor;
    if(lighthouseShaftPass==1)
    {
        float height=ObjectHeight*14.0+7.0;
        if(abs(height-4.4)<=.675 || abs(height-10.2)<=.675)
            baseColor=vec3(.72,.07,.055);
    }
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
        float sandMask=terrainPalette==1?1-smoothstep(13.0,22.0,islandSignedDistance(FragPos.xz)):1;
        baseColor *= mix(vec3(1),mix(vec3(1),texSample.rgb*1.22,.55),sandMask);
    }
    float wetSand=terrainPalette==1?(1-smoothstep(seaLevel+.06,seaLevel+.55,FragPos.y))
        *(1-smoothstep(10.0,18.0,islandSignedDistance(FragPos.xz))):0;
    baseColor*=mix(1.0,.70,wetSand);

    if (InstanceMaterial.z == 1)
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
    int mode=forceShadingMode==0?shadingMode:(forceShadingMode==1?2:forceShadingMode==2?1:0);
    float visibility=sunVisibility(FragPos,Normal);
    if (mode == 1)
    {
        vec3 ambient=ambientStrength*lightColor;
        FragColor=vec4((ambient+(GouraudColor-ambient)*visibility)*baseColor+GouraudSpecular*visibility+nightLighting(baseColor,Normal),objectAlpha);
        return;
    }

    // ========================================================================
    // 2. FLAT SHADING vs. 3. PHONG SHADING
    // Flat: faceted normal computed from screen derivatives (rocks / distant)
    // Phong: smooth interpolated surface normal (robot, umbrella, boat, palms)
    // ========================================================================
    vec3 norm;
    if (mode == 2)
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

    if (boatSailPass == 1 && dot(norm,viewPos-FragPos) < 0.0) norm = -norm;
    if(foliagePass==1 && dot(norm,viewPos-FragPos)<0)norm=-norm;

    // 1. Ambient Lighting
    vec3 ambient = ambientStrength * lightColor;
    if (boatHullPass == 1)
    {
        // Wet paint near the waterline is darker and smoother. A cool sky fill
        // distinguishes the shaded side and avoids flat, uniformly white hulls.
        float wet = 1.0 - smoothstep(0.0, 0.22, FragPos.y - boatWaterHeight);
        baseColor *= mix(1.0, 0.72, wet);
        ambient += vec3(0.12, 0.18, 0.24) * ambientStrength
                 * (0.45 + 0.55 * max(norm.y, 0.0));
    }

    // 2. Diffuse Lighting
    vec3 lightDirection = normalize(lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Specular Lighting (Phong)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDirection, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mix(max(InstanceMaterial.x,1.0),72.0,wetSand));
    vec3 specular = mix(InstanceMaterial.y,.28,wetSand) * spec * lightColor;

    // Combine lighting components with base surface color
    // Dielectric highlights reflect the light color, not the pigment color.
    vec3 result = (ambient + diffuse*visibility) * baseColor + specular*visibility+nightLighting(baseColor,norm);
    FragColor = vec4(result, objectAlpha);
}
)BEACH_GLSL";
inline constexpr char WaterVertex[] = R"BEACH_GLSL(#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out float WaveElevation;
out float WaterDepth;
out float ShoreDistance;
out float WakeFactor;      // How much this fragment is in the boat's wake trail
out float BowWaveFactor;   // How much this fragment is affected by bow wave
out vec2 WaterLighting;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float time;
uniform float seaLevel;
uniform vec3 lightDir;
uniform vec3 viewPos;

// Boat interaction uniforms
uniform vec3  boatPos;
uniform float boatHeading;
uniform float boatSpeed;

// ============================================================================
// GERSTNER OCEAN WAVE SIMULATION (GPU GEMS / PHYSICAL OCEANOGRAPHY)
// Directionally spread gravity-wave spectrum with one dominant swell system.
// Computes horizontal and vertical trochoidal displacement and analytical normals.
// ============================================================================

struct GerstnerWave {
    vec2  direction;
    float wavelength;
    float amplitude;
    float steepness;
    float phaseOffset;
};

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
    vec3 worldPos = vec3(model * vec4(aPos, 1.0));
    
    // A dominant long-period swell with a narrow directional spread. Shorter
    // local wind waves carry much less energy and remain aligned with the same
    // weather system instead of forming a regular cross-hatched interference grid.
    GerstnerWave waves[6];
    waves[0] = GerstnerWave(normalize(vec2(-0.06, -0.998)), 46.0, 0.40, 0.30, 0.0);
    waves[1] = GerstnerWave(normalize(vec2( 0.05, -0.999)), 31.0, 0.23, 0.27, 1.7);
    waves[2] = GerstnerWave(normalize(vec2(-0.13, -0.991)), 21.0, 0.13, 0.22, 3.8);
    waves[3] = GerstnerWave(normalize(vec2( 0.11, -0.994)), 13.0, 0.07, 0.17, 5.1);
    waves[4] = GerstnerWave(normalize(vec2(-0.18, -0.984)),  8.5, 0.04, 0.12, 2.6);
    waves[5] = GerstnerWave(normalize(vec2( 0.17, -0.985)),  5.8, 0.02, 0.08, 4.4);

    vec3 displacement = vec3(0.0);
    vec3 normalAccum = vec3(0.0, 1.0, 0.0);
    float totalElevation = 0.0;

    // Signed island shoreline distance. Water surrounds every side of the
    // footprint; wave amplitude falls smoothly across the shallow beach shelf.
    float islandDistance = islandSignedDistance(worldPos.xz);
    float shoreInset=(seaLevel-.08)/(seaLevel>.08?.055:.12);
    float outsideDistance = max(shoreInset-islandDistance, 0.0);
    float shoreTaper = smoothstep(0.35, 7.0, outsideDistance);
    float shoaling = 1.0 + 0.22
        * (1.0 - smoothstep(15.0, 38.0, outsideDistance))
        * smoothstep(2.0, 9.0, outsideDistance);

    // Shallow-water refraction turns incoming wave energy toward the local
    // shoreline normal. The blend is intentionally limited so the open ocean
    // retains long, coherent swell fronts instead of radial pond ripples.
    vec2 towardIslandVector = vec2(-worldPos.x / (110.0 * 110.0),
                                   -(worldPos.z + 100.0) / (155.0 * 155.0));
    vec2 towardIsland = length(towardIslandVector) > 0.00001
        ? normalize(towardIslandVector) : vec2(0.0, -1.0);
    float refraction = (1.0 - smoothstep(10.0, 48.0, outsideDistance)) * 0.58;

    for (int i = 0; i < 6; ++i)
    {
        float k = 2.0 * 3.14159265 / waves[i].wavelength;
        float omega = sqrt(9.81 * k); // deep-water gravity-wave dispersion
        vec2 refractedDirection = normalize(mix(waves[i].direction, towardIsland, refraction));
        float f = k * dot(refractedDirection, worldPos.xz) - omega * time + waves[i].phaseOffset;
        
        float cosf_val = cos(f);
        float sinf_val = sin(f);

        float amp = waves[i].amplitude * shoreTaper * shoaling;
        float q   = waves[i].steepness;

        // Trochoidal horizontal and vertical Gerstner displacement
        displacement.x += q * amp * refractedDirection.x * cosf_val;
        displacement.y += amp * sinf_val;
        displacement.z += q * amp * refractedDirection.y * cosf_val;

        // Partial derivatives for exact analytical normal calculation
        normalAccum.x -= refractedDirection.x * (k * amp) * cosf_val;
        normalAccum.y -= q * (k * amp) * sinf_val;
        normalAccum.z -= refractedDirection.y * (k * amp) * cosf_val;

        totalElevation += amp * sinf_val;
    }

    // ========================================================================
    // BOAT WAKE & BOW WAVE DISPLACEMENT
    // Creates V-shaped Kelvin wake pattern + bow wave bulge near the boat
    // ========================================================================
    float wakeFact = 0.0;
    float bowFact = 0.0;

    if (abs(boatSpeed) > 0.2)
    {
        float headRad = radians(boatHeading);
        float speed = abs(boatSpeed);
        vec2 boatFwd = vec2(sin(headRad), cos(headRad)) * sign(boatSpeed);
        vec2 boatRight = vec2(cos(headRad), -sin(headRad));

        // Vector from boat to this water vertex
        vec2 toFrag = worldPos.xz - boatPos.xz;
        float longitudinal = dot(toFrag, boatFwd);   // + = ahead of boat, - = behind
        float lateral = dot(toFrag, boatRight);       // Left/right distance
        float dist = length(toFrag);

        // --- Bow Wave: Radial V-shaped displacement ahead and to sides ---
        float bowDist = length(toFrag - boatFwd * 3.65);
        float bowRadius = 1.0 + speed * 0.06;
        bowFact = (1.0 - smoothstep(0.25, bowRadius, bowDist)) * clamp(speed / 6.0, 0.0, 1.0);
        float bowWaveHeight = bowFact * 0.12 * clamp(speed / 12.0, 0.0, 1.0);
        displacement.y += bowWaveHeight;

        // --- Kelvin Wake Pattern: V-shaped trailing wake behind the boat ---
        if (longitudinal < -3.55 && dist < 60.0)
        {
            float aftDist = -longitudinal - 3.55;
            float absLateral = abs(lateral);

            // Kelvin envelope: wake arms spread at ~19.47 degrees (sin(19.47) ≈ 0.333)
            float kelvinAngle = 0.354; // tan(19.47 degrees)
            float wakeArmDist = aftDist * kelvinAngle;

            // Transverse wake oscillation along the V-arms
            float wakeWidth = 0.8 + aftDist * 0.06;  // Wake arms widen with distance
            float armProximity = exp(-pow(absLateral - wakeArmDist, 2.0) / (wakeWidth * wakeWidth));

            // Centerline turbulent wake (directly behind propeller)
            float centerWake = exp(-lateral * lateral / (1.2 + aftDist * 0.04));

            // Distance decay
            float decay = exp(-aftDist * 0.065) * clamp(speed / 5.0, 0.0, 1.0);

            // Oscillating wake waves perpendicular to wake arms
            float wakeFreq = 3.14159 * 2.0 / (3.0 + speed * 0.18);
            float wakePhase = aftDist * wakeFreq;
            float wakeOscillation = sin(wakePhase);

            float wakeContrib = (armProximity * 0.7 + centerWake * 0.5) * decay;
            float wakeHeight = wakeContrib * wakeOscillation * 0.10 * clamp(speed / 12.0,0.0,1.0);

            displacement.y += wakeHeight;
            wakeFact = clamp(wakeContrib * decay * 2.0, 0.0, 1.0);

            // Disturb normals in wake region for choppy appearance
            normalAccum.x += sin(aftDist * 1.5 + time * 3.0) * wakeContrib * 0.15;
            normalAccum.z += cos(aftDist * 2.0 + lateral * 1.8) * wakeContrib * 0.12;
        }

        // --- Displacement depression directly under boat (hull pushes water down) ---
        float underBoat = exp(-dist * dist / 5.0) * clamp(speed / 8.0, 0.0, 1.0);
        displacement.y -= underBoat * 0.08;
    }

    worldPos += displacement;
    worldPos.y+=seaLevel;
    FragPos = worldPos;
    Normal = normalize(normalAccum);
    TexCoord = aTexCoord;
    WaveElevation = totalElevation;
    WakeFactor = wakeFact;
    BowWaveFactor = bowFact;
    vec3 L=normalize(lightDir),V=normalize(viewPos-worldPos);
    WaterLighting=vec2(max(dot(Normal,L),0),pow(max(dot(Normal,normalize(L+V)),0),160));

    // Radial depth from the complete island shoreline.
    WaterDepth = outsideDistance * 0.095;
    ShoreDistance = outsideDistance;

    gl_Position = proj * view * vec4(FragPos, 1.0);
}
)BEACH_GLSL";
inline constexpr char WaterFragment[] = R"BEACH_GLSL(#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in float WaveElevation;
in float WaterDepth;
in float ShoreDistance;
in float WakeFactor;
in float BowWaveFactor;
in vec2 WaterLighting;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 lightDir;       // Direction towards the light source
uniform vec3 viewPos;        // Camera world position
uniform float ambientStrength;
uniform float time;
uniform int forceShadingMode;
uniform sampler2D sunShadowMap;
uniform mat4 sunShadowMatrix;
uniform int sunShadowsEnabled;
uniform int nightLightCount;
uniform vec4 nightLightPositions[20];
uniform vec4 nightLightColors[20];
uniform float nightLightIntensity;

float sunVisibility()
{
    if(sunShadowsEnabled==0)return 1;
    vec4 q=sunShadowMatrix*vec4(FragPos,1);vec3 p=q.xyz/q.w*.5+.5;
    if(p.z<=0 || p.z>=1 || any(lessThan(p.xy,vec2(0))) || any(greaterThan(p.xy,vec2(1))))return 1;
    vec2 texel=1.0/vec2(textureSize(sunShadowMap,0));float v=0;
    for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y)
        v+=p.z-.00010<=texture(sunShadowMap,p.xy+vec2(x,y)*texel).r?1.0:0.0;
    return v/9;
}

// Boat interaction
uniform float boatSpeed;
uniform vec4 hulls[3]; // X, Z, compass heading, model scale
uniform mat4 inverseHulls[3]; // Inverse of the actual pitched/rolled boat pose
uniform int hullMaskEnabled;
uniform vec4 wakeTrail[32]; // X, Z, birth time, speed
uniform int wakeCount;

// Textures
uniform sampler2D foamTex;
uniform int useFoamTex;

// Cross-section of BoatHullMesh, not a rectangular hole in the ocean. A
// fragment inside the enclosed shell must not draw over the cockpit floor.
bool insideHull(vec3 p,bool sail)
{
    float stern=sail?-1.74:-1.97,bow=sail?1.95:2.25;
    if(p.z<=stern || p.z>=bow)return false;
    float t=(p.z-stern)/(bow-stern),entry=pow(t,7.0);
    float w=(sail?0.62:0.73)*(1.0-pow(t,3.4))+0.018;
    float xs[7]=float[7](0.0,0.80,0.96,0.97,0.985,1.0,1.0);
    float ys[7]=float[7](-0.32,-0.075,0.10,0.13,0.18,0.45,0.49);
    float width=0.0;
    for(int b=0;b<6;++b)
    {
        float lo=ys[b]+entry*(b<2?0.40:0.12);
        float hi=ys[b+1]+entry*(b+1<2?0.40:0.12);
        if(p.y>=min(lo,hi) && p.y<=max(lo,hi))
            width=max(width,w*(abs(hi-lo)<0.000001?max(xs[b],xs[b+1]):mix(xs[b],xs[b+1],(p.y-lo)/(hi-lo))));
    }
    return abs(p.x)<width-0.002;
}

// Fine wake normals are evaluated per pixel: the far-reaching ocean mesh
// cannot resolve metre-wide propeller wash with vertex displacement alone.
float wakeGaussian(float x){return exp(-x*x);}
float wakeHeight(vec2 p,out float influence)
{
    influence=0.0;
    float speed=abs(boatSpeed);
    if(speed<=0.2 || hulls[0].w<=0.0)return 0.0;
    vec2 relative=p-hulls[0].xy;
    if(dot(relative,relative)>6400.0)return 0.0;
    float a=radians(hulls[0].z);
    vec2 f=vec2(sin(a),cos(a))*sign(boatSpeed),r=vec2(cos(a),-sin(a));
    float along=dot(relative,f),across=dot(relative,r);
    float trailingEnd=(boatSpeed>0.0?1.97:2.25)*hulls[0].w;
    float aft=-along-trailingEnd;
    if(aft<0.0 || aft>65.0)return 0.0;
    float arm=wakeGaussian((abs(across)-aft*0.354)/(0.55+aft*0.045));
    float wash=wakeGaussian(across/(0.6+aft*0.045));
    float strength=smoothstep(0.2,5.0,speed)*smoothstep(0.0,0.8,aft)*exp(-aft*0.055);
    influence=strength*max(arm,wash);
    return strength*(arm*sin(aft*2.4-time*3.4)*0.065
                   +wash*sin(aft*5.0-time*7.0+sin(across*5.0))*0.035);
}

void main()
{
    if(hullMaskEnabled==1)
    for(int i=0;i<3;++i)
    {
        if(hulls[i].w<=0.0 || distance(FragPos.xz,hulls[i].xy)>5.0*hulls[i].w)continue;
        if(insideHull((inverseHulls[i]*vec4(FragPos,1)).xyz,i>0))discard;
    }
    vec3 N = normalize(Normal);
    int mode=forceShadingMode==0?1:(forceShadingMode==1?2:forceShadingMode==2?1:0);
    if(mode==2){N=normalize(cross(dFdx(FragPos),dFdy(FragPos)));if(N.y<0)N=-N;}
    // Actual seas contain short wind waves and capillary ripples, but their
    // contribution at this scale is primarily a subtle change in reflection,
    // not metre-high repeating geometry.
    float rippleX = sin(dot(FragPos.xz, vec2(0.78, 0.31)) * 1.35 + time * 1.9);
    float rippleZ = sin(dot(FragPos.xz, vec2(-0.22, 0.91)) * 1.72 + time * 2.3);
    if(mode==0)N = normalize(N + vec3(rippleX * 0.018, 0.0, rippleZ * 0.018));
    float wakeInfluence,unused;
    float wake= wakeHeight(FragPos.xz,wakeInfluence);
    float dx=(wakeHeight(FragPos.xz+vec2(0.08,0),unused)-wake)/0.08;
    float dz=(wakeHeight(FragPos.xz+vec2(0,0.08),unused)-wake)/0.08;
    N=normalize(N+vec3(-dx,0,-dz));
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
    float F0 = 0.0204; // Air/water dielectric interface, IOR 1.33.
    float fresnel = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);

    // Sky ambient color reflected on water surface
    vec3 skyReflectColor = mix(vec3(0.35, 0.65, 0.90), lightColor, 0.35);

    // ========================================================================
    // 3. SUN SPECULAR GLINT & BLINN-PHONG HIGHLIGHTS
    // Dynamic glistening across angled wave peaks towards the sun
    // ========================================================================
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, 160.0);
    if(mode==1){NdotL=mix(WaterLighting.x,NdotL,wakeInfluence);spec=mix(WaterLighting.y,spec,wakeInfluence);}
    vec3 specular = lightColor * spec * 1.45 * (0.25 + 0.75 * fresnel);

    // Diffuse / Ambient lighting
    vec3 ambient = ambientStrength * skyReflectColor * waterBodyColor;
    vec3 diffuse = NdotL * lightColor * waterBodyColor * 0.7;

    float visibility=sunVisibility();
    vec3 finalColor = ambient + diffuse*visibility + mix(vec3(0.0), skyReflectColor, fresnel * 0.65) + specular*visibility;

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
    // Per-pixel contact avoids losing narrow foam features in the ocean grid.
    float wakeFoam=0.0, contactShade=0.0, hullReflection=0.0;
    for(int i=0;i<3;++i)
    {
        float scale=hulls[i].w;
        if(scale<=0.0) continue;
        float a=radians(hulls[i].z);
        vec2 fwd=vec2(sin(a),cos(a)),right=vec2(cos(a),-sin(a));
        vec2 relative=FragPos.xz-hulls[i].xy;
        if(length(relative)>(i==0?64.0:12.0)*scale) continue;
        float along=dot(relative,fwd),across=abs(dot(relative,right));
        float stern=(i==0?1.97:1.74)*scale,bow=(i==0?2.25:1.95)*scale;
        float t=clamp((along+stern)/(bow+stern),0.0,1.0);
        float halfWidth=((i==0?0.73:0.62)*(1.0-pow(t,3.4))+0.018)*scale;
        float edge=max(across-halfWidth,max(-along-stern,along-bow));
        contactShade=max(contactShade,(1.0-smoothstep(0.05,0.65,edge))*0.35);
        float contact=exp(-pow(edge/0.13,2.0));
        float lace=0.5+0.5*sin(along*6.0-time*2.4+across*4.0);
        wakeFoam=max(wakeFoam,contact*lace*(i==0?0.06+0.12*clamp(abs(boatSpeed)/6.0,0.0,1.0):0.04));
        hullReflection=max(hullReflection,exp(-max(edge,0.0)*1.5)*smoothstep(0.1,0.8,edge)*(1.0-fresnel)*0.12);
        if(i==0 && abs(boatSpeed)>0.2)
        {
            float speed=abs(boatSpeed),direction=sign(boatSpeed);
            float ahead=along*direction,start=direction>0.0?bow:stern;
            float sideDistance=abs(across-(0.35+max(start-ahead,0.0)*0.25));
            float bowFoam=exp(-pow(sideDistance/0.22,2.0))*(1.0-smoothstep(0.5,2.6,abs(ahead-start+0.65)));
            wakeFoam=max(wakeFoam,bowFoam*clamp(speed/8.0,0.0,0.65));
            float trailingEnd=direction>0.0?stern:bow;
            float aft=max(-ahead-trailingEnd,0.0);
            if(ahead < -trailingEnd && aft<65.0)
            {
                float arm=wakeGaussian((across-aft*0.354)/(0.50+aft*0.045));
                float ridge=0.65+0.35*sin(aft*2.4-time*3.4);
                float wash=wakeGaussian(across/(0.65+aft*0.05));
                float turbulence=0.65+0.35*sin(along*7.0+time*4.0)*cos(across*9.0-time*2.0);
                float strength=smoothstep(0.2,5.0,speed)*smoothstep(0.0,0.6,aft)*exp(-aft*0.065);
                wakeFoam=max(wakeFoam,(arm*ridge*0.42+wash*turbulence*0.68)*strength);
            }
        }
    }
    // Persistent propeller wash follows curved paths and fades after stopping.
    if(length(FragPos.xz-hulls[0].xy)<180.0)
    for(int i=1;i<32;++i)
    {
        if(i>=wakeCount) break;
        vec4 a=wakeTrail[i-1],b=wakeTrail[i];
        vec2 segment=b.xy-a.xy;
        float u=clamp(dot(FragPos.xz-a.xy,segment)/max(dot(segment,segment),0.001),0.0,1.0);
        float age=max(time-mix(a.z,b.z,u),0.0);
        if(age>12.0) continue;
        float speed=mix(a.w,b.w,u);
        float d=length(FragPos.xz-mix(a.xy,b.xy,u));
        float width=0.35+min(age,8.0)*0.13;
        float turbulence=0.65+0.35*sin(FragPos.x*7.0+time*2.0)*cos(FragPos.z*5.0-time*1.3);
        float wash=exp(-pow(d/width,2.0))*exp(-age*0.28)*turbulence;
        wakeFoam=max(wakeFoam,wash*clamp(speed/6.0,0.0,0.78));
    }
    finalColor*=1.0-contactShade;
    finalColor=mix(finalColor,vec3(0.12,0.18,0.22)*lightColor,hullReflection);

    float totalFoam = clamp(crestFoam * 0.82 + shoreFoam * 1.05 + wakeFoam * 1.25, 0.0, 1.0);
    vec3 foamColor = vec3(0.96, 0.98, 1.0) * clamp(lightColor * (ambientStrength + 0.70),0.0,1.0);

    // Wake foam has a slight blue-white tint (aerated sea water)
    vec3 wakeFoamTint = foamColor * vec3(0.90, 0.96, 1.0);
    vec3 blendedFoamColor = mix(foamColor, wakeFoamTint, clamp(wakeFoam / (totalFoam + 0.001), 0.0, 1.0));

    finalColor = mix(finalColor, blendedFoamColor, totalFoam * 0.88);
    if(nightLightIntensity>0)
    for(int i=0;i<20;++i)
    {
        if(i>=nightLightCount)break;
        vec3 delta=nightLightPositions[i].xyz-FragPos;float d=length(delta),radius=nightLightPositions[i].w;
        if(d>=radius)continue;
        vec3 lamp=delta/max(d,.01);
        float falloff=(1-smoothstep(radius*.65,radius,d))/(1+.12*d+.055*d*d);
        float glint=pow(max(dot(N,normalize(lamp+V)),0),64);
        finalColor+=nightLightColors[i].rgb*nightLightColors[i].w*falloff*nightLightIntensity*(waterBodyColor*.25*max(dot(N,lamp),0)+glint*.5);
    }

    // ========================================================================
    // 6. ALPHA TRANSPARENCY
    // Shallow water near the shore is translucent to show golden sand beneath
    // Wave foam and deeper offshore water are progressively more opaque
    // ========================================================================
    float alpha = mix(0.82, 1.0, clamp(WaterDepth / 1.5, 0.0, 1.0));
    alpha = max(alpha, totalFoam * 0.95);

    FragColor = vec4(finalColor, alpha);
}
)BEACH_GLSL";
inline constexpr char SkyboxVertex[] = R"BEACH_GLSL(#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 proj;

void main()
{
    TexCoords = aPos;
    // Strip translation from view matrix so skybox stays centered on camera
    mat4 staticView = mat4(mat3(view));
    vec4 pos = proj * staticView * vec4(aPos, 1.0);
    // Setting z = w places the skybox at the far clipping plane (depth = 1.0)
    gl_Position = pos.xyww;
}
)BEACH_GLSL";
inline constexpr char SkyboxFragment[] = R"BEACH_GLSL(#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 zenithColor;
uniform vec3 horizonColor;
uniform vec3 oceanHorizonColor;
uniform vec3 sunDir;
uniform vec3 sunGlowColor;
uniform float timeOfDay; // 0.0 to 24.0

void main()
{
    vec3 dir = normalize(TexCoords);

    vec3 skyCol;
    if (dir.y >= -0.04)
    {
        // Sky hemisphere: smooth gradient from horizon to zenith
        float t = pow(clamp(dir.y + 0.04, 0.0, 1.0), 0.48);
        skyCol = mix(horizonColor, zenithColor, t);

        // ====================================================================
        // PHOTOREALISTIC CELESTIAL SUN & ATMOSPHERIC CORONA (AT INFINITY)
        // Truly distant celestial star with limb darkening and Rayleigh/Mie halo
        // ====================================================================
        vec3 sDir = normalize(sunDir);
        float cosTheta = dot(dir, sDir);

        if (sDir.y > -0.10 && cosTheta > 0.0)
        {
            // 1. Broad forward atmospheric Mie scattering / solar glare
            float wideHaze = pow(cosTheta, 6.0) * 0.35;
            float midAureole = pow(cosTheta, 42.0) * 0.90;
            float innerCorona = pow(cosTheta, 280.0) * 2.4;

            // 2. Physical Solar Disc with natural atmospheric limb darkening
            // Angular diameter ~ 0.53 degrees -> cos ~ 0.9999
            // With subtle atmospheric optical expansion near horizon:
            float discThresh = (timeOfDay < 7.5 || timeOfDay > 16.5) ? 0.9980 : 0.9988;
            float discSoftEdge = 0.0005;
            float discMask = smoothstep(discThresh - discSoftEdge, discThresh, cosTheta);

            // Blinding incandescent white-gold solar core
            float limbDarkening = 0.45 + 0.55 * smoothstep(discThresh, 1.0, cosTheta);
            vec3 coreIncandescent = vec3(1.0, 0.99, 0.95);
            vec3 discColor = mix(sunGlowColor, coreIncandescent, 0.85 + 0.15 * limbDarkening) * 3.8;

            vec3 coronaGlow = sunGlowColor * (wideHaze + midAureole + innerCorona);
            skyCol += coronaGlow + discColor * discMask;
        }

        // Night sky twinkling stars (visible when sun is down)
        if (timeOfDay < 5.5 || timeOfDay > 19.2)
        {
            float nightFactor = 1.0;
            if (timeOfDay >= 5.0 && timeOfDay <= 5.8) nightFactor = (5.8 - timeOfDay) / 0.8;
            if (timeOfDay >= 18.8 && timeOfDay <= 19.8) nightFactor = (timeOfDay - 18.8) / 1.0;

            vec3 p = floor(dir * 180.0);
            float starHash = fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
            if (starHash > 0.993 && dir.y > 0.08)
            {
                float brightness = (starHash - 0.993) / 0.007;
                skyCol += vec3(0.88, 0.92, 1.0) * brightness * nightFactor * 1.1;
            }
        }
    }
    else
    {
        // Seaward horizon haze transition
        float t = pow(clamp(-dir.y * 3.5, 0.0, 1.0), 0.65);
        skyCol = mix(horizonColor, oceanHorizonColor, t);
    }

    FragColor = vec4(skyCol, 1.0);
}
)BEACH_GLSL";
inline const char* source(std::string_view name)
{
    if(name=="default.vert")return DefaultVertex;
    if(name=="default.frag")return DefaultFragment;
    if(name=="water.vert")return WaterVertex;
    if(name=="water.frag")return WaterFragment;
    if(name=="skybox.vert")return SkyboxVertex;
    if(name=="skybox.frag")return SkyboxFragment;
    throw std::runtime_error("Unknown embedded shader: "+std::string(name));
}
} // namespace BeachDayShaders

// ============================================================================
// SECTION: Shader compilation and lifecycle
// ============================================================================
#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

using namespace std;

string get_file_contents(const char* filename);

class Shader {
public:
	GLuint ID;
	mutable std::unordered_map<std::string,GLint> uniformLocations;
	GLint location(const char* name) const {
	    auto found=uniformLocations.find(name);
	    if(found!=uniformLocations.end())return found->second;
	    GLint result=glGetUniformLocation(ID,name);
	    uniformLocations.emplace(name,result);return result;
	}
	Shader(const char* vertexFile, const char* fragmentFile);

	void Activate();
	void Delete();

private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);
};

#endif
inline string get_file_contents(const char* name)
{
    return BeachDayShaders::source(name);
}
inline Shader::Shader(const char* vertexFile, const char* fragmentFile) {

	string vertexCode = get_file_contents(vertexFile);
	string fragmentCode = get_file_contents(fragmentFile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	compileErrors(vertexShader, "VERTEX");

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	compileErrors(fragmentShader, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);
	compileErrors(ID, "PROGRAM");

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

inline void Shader::Activate() {
	glUseProgram(ID);
}

inline void Shader::Delete() {
	glDeleteProgram(ID);
	uniformLocations.clear();ID=0;
}

// Checks if the different Shaders have compiled properly
inline void Shader::compileErrors(unsigned int shader, const char* type)
{
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];
	if (std::string(type) != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}

// ============================================================================
// SECTION: TextureManager.h
// ============================================================================
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

// ============================================================================
// SECTION: Skybox.h
// ============================================================================
#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
        glUniformMatrix4fv(shader.location("view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(shader.location("proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(shader.location("zenithColor"), 1, glm::value_ptr(zenithColor));
        glUniform3fv(shader.location("horizonColor"), 1, glm::value_ptr(horizonColor));
        glUniform3fv(shader.location("oceanHorizonColor"), 1, glm::value_ptr(oceanHorizonColor));
        glUniform3fv(shader.location("sunDir"), 1, glm::value_ptr(sunDir));
        glUniform3fv(shader.location("sunGlowColor"), 1, glm::value_ptr(sunGlowColor));
        glUniform1f(shader.location("timeOfDay"), timeOfDay);

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

// ============================================================================
// SECTION: SunShadows.h
// ============================================================================
#ifndef SUN_SHADOWS_H
#define SUN_SHADOWS_H
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

class SunShadows
{
    GLuint framebuffer=0,depth=0;
public:
    static constexpr int resolution=4096;
    glm::mat4 matrix=glm::mat4(1);
    void init()
    {
        glGenFramebuffers(1,&framebuffer);glGenTextures(1,&depth);
        glBindTexture(GL_TEXTURE_2D,depth);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,resolution,resolution,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
        const float border[]={1,1,1,1};glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,border);
        glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depth,0);
        glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Sun shadow framebuffer is incomplete");
        glBindFramebuffer(GL_FRAMEBUFFER,0);
    }
    void begin(Shader& shader,glm::vec3 direction,float span=300,glm::vec3 center=glm::vec3(0,20,-65))
    {
        glm::mat4 view=glm::lookAt(center+direction*700.0f,center,glm::vec3(0,1,0));
        glm::mat4 projection=glm::ortho(-span,span,-span,span,10.0f,1500.0f);
        matrix=projection*view;
        // Snap the light-space origin to texels to reduce moving-camera shimmer.
        glm::vec4 origin=matrix*glm::vec4(0,0,0,1);
        glm::vec2 grid=glm::vec2(origin)*(resolution*.5f);
        glm::vec2 correction=(glm::round(grid)-grid)*(2.0f/resolution);
        matrix[3][0]+=correction.x;matrix[3][1]+=correction.y;
        projection=matrix*glm::inverse(view);
        glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_2D,0);glActiveTexture(GL_TEXTURE0);
        glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);glViewport(0,0,resolution,resolution);
        glClear(GL_DEPTH_BUFFER_BIT);glDisable(GL_BLEND);
        glEnable(GL_POLYGON_OFFSET_FILL);glPolygonOffset(1.5f,2.0f);
        shader.Activate();glUniform1i(shader.location("shadowDepthPass"),1);
        glUniformMatrix4fv(shader.location("view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(shader.location("proj"),1,GL_FALSE,glm::value_ptr(projection));
    }
    void end(Shader& shader,int width,int height,const glm::mat4& view,const glm::mat4& projection)
    {
        glDisable(GL_POLYGON_OFFSET_FILL);glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,width,height);
        shader.Activate();glUniform1i(shader.location("shadowDepthPass"),0);
        glUniformMatrix4fv(shader.location("view"),1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(shader.location("proj"),1,GL_FALSE,glm::value_ptr(projection));
    }
    void bind(Shader& shader,bool enabled) const
    {
        shader.Activate();glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_2D,depth);glActiveTexture(GL_TEXTURE0);
        glUniform1i(shader.location("sunShadowMap"),3);
        glUniform1i(shader.location("sunShadowsEnabled"),enabled);
        glUniformMatrix4fv(shader.location("sunShadowMatrix"),1,GL_FALSE,glm::value_ptr(matrix));
    }
    void cleanup(){glDeleteTextures(1,&depth);glDeleteFramebuffers(1,&framebuffer);}
};
#endif

// ============================================================================
// SECTION: DayCyclePanel.h
// ============================================================================
#ifndef DAY_CYCLE_PANEL_H
#define DAY_CYCLE_PANEL_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>
#include <stdexcept>

// Lightweight screen-space controls, in GLFW logical pixels (HiDPI-safe).
// Input is captured before the camera receives mouse events.
enum ObjectGuideSection { GUIDE_SEATED,GUIDE_WALKING,GUIDE_UMBRELLA,GUIDE_BOAT,
    GUIDE_ATV,GUIDE_LIGHTHOUSE,GUIDE_DAY,GUIDE_ENVIRONMENT,GUIDE_CAMERA };
struct ObjectGuideEntry {std::string title;std::vector<std::string> instructions;};
inline const std::vector<ObjectGuideEntry>& objectGuideEntries(){
    static const std::vector<ObjectGuideEntry> entries={
        {"SEATED ROBOT",{"C / 7 - FOCUS BEACH","1 / 2 - TURN HEAD","RESTING ON THE LOUNGER"}},
        {"WALKING ROBOT",{"ENTER - CONTROL / EXIT","W / S - WALK / BACK","A / D - TURN; ARROWS TOO","FOLLOW THE DRY BEACH PATH","REVERSE OR TURN IF BLOCKED"}},
        {"UMBRELLA",{"C - FOCUS BEACH","I - OPEN / CLOSE","3 / 4 - CHANGE POLE TILT","PRESS I AGAIN TO REVERSE","WORKS WHILE DAY IS PAUSED"}},
        {"MOTORBOAT",{"B / 5 - FOCUS BOAT","F - PILOT / EXIT","W / S - THROTTLE / BRAKE","A / D - STEER; ARROWS TOO","MOVEMENT CREATES A WAKE"}},
        {"ATV AND CHARGER",{"H - RIDE / EXIT","W / S - DRIVE / REVERSE","A / D - STEER; ARROWS TOO","SPACE - BRAKE","U - START / STOP TOUR","E - CHASE / RIDER VIEW","Q - CONNECT / DISCONNECT","STOP WITHIN 2 M OF CHARGER","STOP TOUR BEFORE CHARGING","DISCONNECT BEFORE DRIVING"}},
        {"LIGHTHOUSE",{"T / 6 - FOCUS TOWER","F9 - PAUSE / RESUME SWEEP","F10 - RESET BEACON","COMMA / PERIOD - AIM","SEMICOLON / APOSTROPHE","CHANGE ROTATION SPEED","F11 / F12 - AIM UP / DOWN","MANUAL AIM PAUSES SWEEP","LIGHT TURNS ON AT NIGHT"}},
        {"DAY AND LIGHTING",{"DRAG DAY SPEED SLIDER","CLICK PAUSE / PLAY","[ / ] - CHANGE DAY SPEED","SPACE - DAY PAUSE","OUTSIDE ATV DRIVING MODE","F5 / J - SUNRISE","F6 / K - MIDDAY","F7 - SUNSET; F8 / N - NIGHT","PATH LIGHTS FADE AT DUSK"}},
        {"ENVIRONMENT OBJECTS",{"P / 8 - PALMS AND ROCKS","Y - VIEW STILT HOUSE","L / 9 - VIEW SAILBOATS","CHAIRS AND HOUSE: VIEW ONLY","TREES SWAY AUTOMATICALLY","COCONUTS FALL AUTOMATICALLY","WAVES AND BOAT BOBBING: AUTO","DISTANT BOATS DRIFT: AUTO","O / R - ISLAND OVERVIEW"}},
        {"CAMERA AND GRAPHICS",{"LEFT DRAG - ORBIT / LOOK","RIGHT DRAG - PAN FREE VIEW","SCROLL - ZOOM FREE VIEW","TAB - CYCLE OBJECT VIEWS","M - CHANGE SHADING","X - TEXTURES; G - SHADOWS","Z - WIREFRAME","F1 - SHOW / HIDE THIS GUIDE","F2 - FPS AND GPU DETAILS","ESC - EXIT PROJECT"}}
    };return entries;
}
struct ObjectGuideRow {std::string text;int section=-1;};
struct ObjectGuideState {
    bool visible=true;int selected=GUIDE_CAMERA,context=-1,capturedButton=-1;
    float left=18,top=18,width=340,height=0,scale=1,scroll=0;
    std::vector<ObjectGuideRow> rows;
    void rebuild(){
        rows.clear();int columns=std::max(8,(int)((width/scale-20)/12));
        auto wrap=[&](std::string value,int section){
            while((int)value.size()>columns){size_t end=value.rfind(' ',columns);
                if(end==std::string::npos || end==0)end=columns;
                rows.push_back({value.substr(0,end),section});value.erase(0,end);
                while(!value.empty()&&value.front()==' ')value.erase(0,1);}
            rows.push_back({value,section});
        };
        const auto& entries=objectGuideEntries();
        for(int i=0;i<(int)entries.size();++i){wrap(std::string(i==selected?"- ":"+ ")+entries[i].title,i);
            if(i==selected)for(const auto& line:entries[i].instructions)wrap(line,-1);}
        scroll=std::clamp(scroll,0.f,std::max(0.f,(float)rows.size()*20-std::max(0.f,height/scale-42)));
    }
    void layout(int w,int h){
        scale=std::clamp((w-36.f)/340.f,.35f,1.f);
        width=std::max(0.f,std::min(340*scale,w*.44f));
        float available=std::max(0.f,h-18-102*scale-12-top);
        height=visible?available:std::min(32*scale,available);rebuild();
    }
    void observe(int current){if(current!=context){context=current;selected=current;scroll=0;rebuild();}}
    void toggle(){visible=!visible;capturedButton=-1;}
    bool contains(double x,double y)const{return height>0&&x>=left&&x<=left+width&&y>=top&&y<=top+height;}
    bool active()const{return capturedButton!=-1;}
    bool mouse(int button,int action,double x,double y){
        if(action==GLFW_RELEASE){if(button==capturedButton){capturedButton=-1;return true;}return false;}
        if(action!=GLFW_PRESS||!contains(x,y))return false;
        capturedButton=button;if(button!=GLFW_MOUSE_BUTTON_LEFT)return true;
        float localY=((float)y-top)/scale;
        if(localY<32){visible=!visible;return true;}
        if(visible&&localY>=42){int row=(int)((localY-42+scroll)/20);
            if(row>=0&&row<(int)rows.size()&&rows[row].section>=0){int section=rows[row].section;
                selected=selected==section?-1:section;rebuild();}}
        return true;
    }
    bool wheel(double x,double y,double amount){
        if(!active()&&!contains(x,y))return false;
        scroll-=static_cast<float>(amount)*48;rebuild();return true;
    }
};

class DayCyclePanel
{
    GLuint program=0,vao=0,vbo=0;
    GLint screenLocation=-1;
    std::vector<float> vertices;
    float left=18,top=18,width=340,scale=1;
    bool captured=false,dragging=false;
    bool clipGuide=false;float clipTop=0,clipBottom=0;
    void rect(float x,float y,float w,float h,float r,float g,float b,float a=1)
    {
        if(clipGuide){float end=std::min(y+h,clipBottom);y=std::max(y,clipTop);h=end-y;if(h<=0)return;}
        for(int corner:{0,1,2,0,2,3})
        {
            float px=x+((corner==1 || corner==2)?w:0),py=y+(corner>=2?h:0);
            vertices.insert(vertices.end(),{left+px*scale,top+py*scale,r,g,b,a});
        }
    }
    static const char* glyph(char c)
    {
        switch(c)
        {
        case 'A':return "01110100011000111111100011000110001";
        case 'B':return "11110100011000111110100011000111110";
        case 'C':return "01111100001000010000100001000001111";
        case 'D':return "11110100011000110001100011000111110";
        case 'E':return "11111100001000011110100001000011111";
        case 'F':return "11111100001000011110100001000010000";
        case 'G':return "01111100001000010111100011000101111";
        case 'H':return "10001100011000111111100011000110001";
        case 'I':return "11111001000010000100001000010011111";
        case 'J':return "00111000100001000010100101001001100";
        case 'K':return "10001100101010011000101001001010001";
        case 'L':return "10000100001000010000100001000011111";
        case 'M':return "10001110111010110101100011000110001";
        case 'N':return "10001110011010110011100011000110001";
        case 'O':return "01110100011000110001100011000101110";
        case 'P':return "11110100011000111110100001000010000";
        case 'Q':return "01110100011000110001101011001001101";
        case 'R':return "11110100011000111110101001001010001";
        case 'S':return "01111100001000001110000010000111110";
        case 'T':return "11111001000010000100001000010000100";
        case 'U':return "10001100011000110001100011000101110";
        case 'V':return "10001100011000110001100010101000100";
        case 'W':return "10001100011000110101101011010101010";
        case 'X':return "10001100010101000100010101000110001";
        case 'Y':return "10001100010101000100001000010000100";
        case 'Z':return "11111000010001000100010001000011111";
        case ':':return "00000011000110000000011000110000000";
        case '-':return "00000000000000011111000000000000000";
        case '%':return "11001110100001000100010001001100011";
        case '[':return "01100010000100001000010000100001100";
        case ']':return "00110000100001000010000100001000110";
        case '+':return "00000001000010011111001000010000000";
        case '(':return "00010001000100001000010000010000010";
        case ')':return "01000001000001000010000100010001000";
        case ',':return "00000000000000000000001100010001000";
        case '\'':return "00100001000010000000000000000000000";
        case '_':return "00000000000000000000000000000011111";
        case '0':return "01110100011001110101110011000101110";
        case '1':return "00100011000010000100001000010001110";
        case '2':return "01110100010000100010001000100011111";
        case '3':return "11110000010000101110000010000111110";
        case '4':return "00010001100101010010111110001000010";
        case '5':return "11111100001000011110000010000111110";
        case '6':return "01110100001000011110100011000101110";
        case '7':return "11111000010001000100010000100001000";
        case '8':return "01110100011000101110100011000101110";
        case '9':return "01110100011000101111000010000101110";
        case '.':return "00000000000000000000000000011000110";
        case '/':return "00001000100001000100010000100010000";
        default:return "00000000000000000000000000000000000";
        }
    }
    void text(float x,float y,const std::string& value,float r=.86f,float g=.93f,float b=.95f)
    {
        for(char c:value)
        {
            const char* bitmap=glyph(c);
            for(int row=0;row<7;++row)for(int col=0;col<5;++col)
                if(bitmap[row*5+col]=='1')rect(x+col*2,y+row*2,2,2,r,g,b);
            x+=12;
        }
    }
    void aim(double x,float& speed)const
    {speed=.05f+3.95f*std::clamp(((float)x-left)/scale-18.0f,0.0f,304.0f)/304.0f;}
public:
    void layout(int windowWidth,int windowHeight)
    {
        scale=std::clamp((windowWidth-36.0f)/340.0f,.35f,1.0f);
        width=340*scale;top=std::max(0.0f,windowHeight-18-102*scale);
    }
    bool contains(double x,double y)const
    {return x>=left && x<=left+width && y>=top && y<=top+102*scale;}
    bool active()const{return captured;}
    bool mouse(int button,int action,double x,double y,float& speed,bool& paused)
    {
        if(button!=GLFW_MOUSE_BUTTON_LEFT)return action==GLFW_PRESS && contains(x,y);
        if(action==GLFW_RELEASE){bool consumed=captured;captured=dragging=false;return consumed;}
        if(action!=GLFW_PRESS || !contains(x,y))return false;
        captured=true;
        float px=((float)x-left)/scale,py=((float)y-top)/scale;
        if(px>=252 && py<=39)paused=!paused;
        else if(py>=41 && py<=79){dragging=true;aim(x,speed);}
        return true;
    }
    bool move(double x,float& speed)
    {if(dragging)aim(x,speed);return captured;}
    void cancel(){captured=dragging=false;}
    void init()
    {
        const char* vs="#version 330 core\nlayout(location=0) in vec2 p;layout(location=1) in vec4 c;uniform vec2 screen;out vec4 color;void main(){color=c;gl_Position=vec4(p.x/screen.x*2-1,1-p.y/screen.y*2,0,1);}";
        const char* fs="#version 330 core\nin vec4 color;out vec4 frag;void main(){frag=color;}";
        auto compile=[](GLenum type,const char* source){GLuint id=glCreateShader(type);glShaderSource(id,1,&source,nullptr);glCompileShader(id);GLint ok;glGetShaderiv(id,GL_COMPILE_STATUS,&ok);if(!ok)throw std::runtime_error("Day slider shader failed");return id;};
        GLuint vertex=compile(GL_VERTEX_SHADER,vs),fragment=compile(GL_FRAGMENT_SHADER,fs);
        program=glCreateProgram();glAttachShader(program,vertex);glAttachShader(program,fragment);glLinkProgram(program);glDeleteShader(vertex);glDeleteShader(fragment);
        GLint ok;glGetProgramiv(program,GL_LINK_STATUS,&ok);if(!ok)throw std::runtime_error("Day slider linking failed");
        screenLocation=glGetUniformLocation(program,"screen");
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),nullptr);glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(2*sizeof(float)));glEnableVertexAttribArray(1);glBindVertexArray(0);
    }
    void draw(int windowWidth,int windowHeight,float speed,bool paused,const std::vector<std::string>& hud={},ObjectGuideState* guide=nullptr)
    {
        layout(windowWidth,windowHeight);vertices.clear();
        rect(0,0,340,102,.025f,.06f,.08f,.93f);rect(0,0,340,2,.17f,.77f,.66f);
        char label[32];std::snprintf(label,sizeof(label),"DAY SPEED %.2f H/S",speed);text(18,17,label);
        rect(252,9,76,30,.12f,.23f,.26f);text(260,17,paused?"PLAY":"PAUSE");
        float t=std::clamp((speed-.05f)/3.95f,0.0f,1.0f);
        rect(18,56,304,5,.20f,.31f,.34f);rect(18,56,304*t,5,.17f,.77f,.66f);
        rect(18+304*t-5,49,10,19,.91f,.98f,.96f);
        text(18,81,"SLOW",.55f,.69f,.71f);text(274,81,"FAST",.55f,.69f,.71f);
        // Draw the read-only HUD above the interactive slider. Wrap long GPU
        // names and hide overflow rather than covering the controls on resize.
        float savedTop=top,savedLeft=left;
        if(guide){guide->layout(windowWidth,windowHeight);left=guide->left;top=guide->top;
            if(guide->height>=20*scale){rect(0,0,guide->width/scale,guide->height/scale,.025f,.06f,.08f,.92f);
                rect(0,0,guide->width/scale,2,.17f,.77f,.66f);
                std::string heading=guide->width/scale>=300?"F1 OBJECT GUIDE":"F1 GUIDE";text(10,9,heading);
                if(guide->visible){clipGuide=true;clipTop=34;clipBottom=guide->height/scale;
                    for(int i=0;i<(int)guide->rows.size();++i){float y=42+i*20-guide->scroll;
                        if(y+18<clipTop||y>clipBottom)continue;
                        const auto& row=guide->rows[i];
                        if(row.section>=0)rect(6,y-2,guide->width/scale-12,20,.10f,.22f,.25f,.95f);
                        text(10,y,row.text,row.section>=0?.42f:.86f,row.section>=0?.91f:.93f,.85f);
                    }clipGuide=false;
                    float viewport=std::max(0.f,guide->height/scale-42),content=guide->rows.size()*20.f;
                    if(content>viewport&&viewport>0){
                        float thumb=std::max(12.f,viewport*viewport/content);
                        rect(guide->width/scale-4,42,3,viewport,.12f,.22f,.25f);
                        rect(guide->width/scale-4,42+(viewport-thumb)*guide->scroll/(content-viewport),3,thumb,.32f,.78f,.72f);
                    }}
            }
        }
        left=guide?guide->left+guide->width+12:savedLeft;
        int columns=std::max(8,(int)((windowWidth-left-20)/(12*scale)));
        std::vector<std::string> rows;
        for(auto line:hud){for(char& c:line)if(c>='a' && c<='z')c-=32;
            while((int)line.size()>columns){rows.push_back(line.substr(0,columns));line.erase(0,columns);}rows.push_back(line);}
        int count=std::min((int)rows.size(),std::max(0,(int)((savedTop-36)/(18*scale))));
        top=18;if(count){rect(0,0,(windowWidth-left-18)/scale,12+18*count,.025f,.06f,.08f,.85f);
            for(int i=0;i<count;++i)text(10,6+18*i,rows[i]);}top=savedTop;left=savedLeft;
        GLint oldProgram,oldVAO,oldBuffer,polygon[2],src,dst,srcAlpha,dstAlpha;
        glGetIntegerv(GL_CURRENT_PROGRAM,&oldProgram);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldVAO);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldBuffer);
        glGetIntegerv(GL_POLYGON_MODE,polygon);glGetIntegerv(GL_BLEND_SRC_RGB,&src);glGetIntegerv(GL_BLEND_DST_RGB,&dst);glGetIntegerv(GL_BLEND_SRC_ALPHA,&srcAlpha);glGetIntegerv(GL_BLEND_DST_ALPHA,&dstAlpha);
        GLboolean depth=glIsEnabled(GL_DEPTH_TEST),blend=glIsEnabled(GL_BLEND),cull=glIsEnabled(GL_CULL_FACE),mask;
        glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);
        glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glDepthMask(GL_FALSE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
        glUseProgram(program);glUniform2f(screenLocation,(float)windowWidth,(float)windowHeight);
        glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(float),vertices.data(),GL_STREAM_DRAW);glDrawArrays(GL_TRIANGLES,0,(GLsizei)vertices.size()/6);
        glBindVertexArray(oldVAO);glBindBuffer(GL_ARRAY_BUFFER,oldBuffer);glUseProgram(oldProgram);glPolygonMode(GL_FRONT_AND_BACK,polygon[0]);glDepthMask(mask);
        if(depth)glEnable(GL_DEPTH_TEST);if(cull)glEnable(GL_CULL_FACE);if(!blend)glDisable(GL_BLEND);glBlendFuncSeparate(src,dst,srcAlpha,dstAlpha);
    }
    void cleanup(){glDeleteVertexArrays(1,&vao);glDeleteBuffers(1,&vbo);glDeleteProgram(program);}
};
#endif

#endif // BEACH_DAY_RENDERING_H
