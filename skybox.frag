#version 330 core
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
