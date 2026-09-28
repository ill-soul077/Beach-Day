#version 330 core

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

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float time;

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
    float outsideDistance = max(-islandDistance, 0.0);
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

    if (boatSpeed > 0.2)
    {
        float headRad = radians(boatHeading);
        vec2 boatFwd = vec2(sin(headRad), cos(headRad));
        vec2 boatRight = vec2(cos(headRad), -sin(headRad));

        // Vector from boat to this water vertex
        vec2 toFrag = worldPos.xz - boatPos.xz;
        float longitudinal = dot(toFrag, boatFwd);   // + = ahead of boat, - = behind
        float lateral = dot(toFrag, boatRight);       // Left/right distance
        float dist = length(toFrag);

        // --- Bow Wave: Radial V-shaped displacement ahead and to sides ---
        float bowDist = length(toFrag - boatFwd * 2.5); // From bow (2.5m ahead of center)
        float bowRadius = 1.5 + boatSpeed * 0.2;
        bowFact = smoothstep(bowRadius, 0.8, bowDist) * clamp(boatSpeed / 6.0, 0.0, 1.0);
        float bowWaveHeight = bowFact * 0.25 * (boatSpeed / 12.0);
        displacement.y += bowWaveHeight;

        // --- Kelvin Wake Pattern: V-shaped trailing wake behind the boat ---
        if (longitudinal < -1.5 && dist < 60.0)
        {
            float aftDist = -longitudinal; // How far behind the boat
            float absLateral = abs(lateral);

            // Kelvin envelope: wake arms spread at ~19.47 degrees (sin(19.47) ≈ 0.333)
            float kelvinAngle = 0.333;
            float wakeArmDist = aftDist * kelvinAngle;

            // Transverse wake oscillation along the V-arms
            float wakeWidth = 0.8 + aftDist * 0.06;  // Wake arms widen with distance
            float armProximity = exp(-pow(absLateral - wakeArmDist, 2.0) / (wakeWidth * wakeWidth));

            // Centerline turbulent wake (directly behind propeller)
            float centerWake = exp(-lateral * lateral / (1.2 + aftDist * 0.04));

            // Distance decay
            float decay = exp(-aftDist * 0.035) * clamp(boatSpeed / 4.0, 0.0, 1.0);

            // Oscillating wake waves perpendicular to wake arms
            float wakeFreq = 3.14159 * 2.0 / (2.5 + boatSpeed * 0.15);
            float wakePhase = aftDist * wakeFreq - time * boatSpeed * 0.8;
            float wakeOscillation = sin(wakePhase) * 0.5 + 0.5;

            float wakeContrib = (armProximity * 0.7 + centerWake * 0.5) * decay;
            float wakeHeight = wakeContrib * wakeOscillation * 0.18 * (boatSpeed / 12.0);

            displacement.y += wakeHeight;
            wakeFact = clamp(wakeContrib * decay * 2.0, 0.0, 1.0);

            // Disturb normals in wake region for choppy appearance
            normalAccum.x += sin(aftDist * 1.5 + time * 3.0) * wakeContrib * 0.15;
            normalAccum.z += cos(aftDist * 2.0 + lateral * 1.8) * wakeContrib * 0.12;
        }

        // --- Displacement depression directly under boat (hull pushes water down) ---
        float underBoat = exp(-dist * dist / 5.0) * clamp(boatSpeed / 8.0, 0.0, 1.0);
        displacement.y -= underBoat * 0.08;
    }

    worldPos += displacement;
    FragPos = worldPos;
    Normal = normalize(normalAccum);
    TexCoord = aTexCoord;
    WaveElevation = totalElevation;
    WakeFactor = wakeFact;
    BowWaveFactor = bowFact;

    // Radial depth from the complete island shoreline.
    WaterDepth = outsideDistance * 0.095;
    ShoreDistance = outsideDistance;

    gl_Position = proj * view * vec4(FragPos, 1.0);
}
