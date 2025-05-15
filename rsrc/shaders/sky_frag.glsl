#version 330 core
#define MAX_LIGHTS 4

in vec3 tex_coord;
in vec3 fragPos;

uniform vec3 camPos;
uniform bool dither;
uniform bool showSpecular;
uniform vec3 lightDir[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightPos[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightColor[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1));
uniform float lightCelestialRadius[MAX_LIGHTS] = float[MAX_LIGHTS](0.0, 0.0, 0.0, 0.0);
uniform bool lightApplySpecular[MAX_LIGHTS] = bool[MAX_LIGHTS](false, false, false, false);
uniform vec3 skyColor;
uniform vec3 horizonColor;
uniform vec3 groundColor;
uniform vec3 groundSpec;
uniform float groundShininess;
uniform float lowAlt = 0;
uniform float highAlt = .05;
uniform float hazeDensity = 0;
uniform float maxHazeDist = 180.0;

out vec4 color;

vec3 calculate_celestial_fog(vec3 hazeColor, int i) {
    float celestialIntensity = length(lightDir[i]);
    if (lightCelestialRadius[i] == 0.0 || celestialIntensity == 0) return hazeColor;
    float celestialAmount = max(dot(normalize(fragPos - camPos), normalize(-lightDir[i])), 0.0);
    return mix(hazeColor, lightColor[i] * celestialIntensity, pow(celestialAmount, ((1 - celestialIntensity) * 200.0) + 8.0));
}

vec3 apply_fog(vec3 color, float dist) {
    vec3 hazeColor = mix(skyColor, horizonColor, max(highAlt * 2, 1.0));
    for (int i = 0; i < MAX_LIGHTS; i++) {
        hazeColor = calculate_celestial_fog(hazeColor, i);
    }
    vec3 extColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
    vec3 insColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
    return color * extColor + hazeColor * (1.0 - insColor);
}

vec3 draw_celestial(vec3 inColor, float phi, float gradientHeight, int i) {
    if (lightCelestialRadius[i] == 0.0 || phi <= 0.0) return inColor;
    vec3 offsetFragPos = normalize(camPos - fragPos) * -1000;
    float dist = distance(offsetFragPos, lightPos[i]);
    return mix(
        inColor,
        mix(
            lightColor[i],
            lightColor[i] * gradientHeight + horizonColor * (1.0 - gradientHeight),
            float(phi < highAlt)
        ),
        float(dist <= lightCelestialRadius[i])
    );
}

vec3 spec_light(int i) {
    if (!lightApplySpecular[i] || groundShininess == 0) return vec3(0);
    vec3 viewRay = normalize(camPos - fragPos);
    vec3 lightRay = normalize((lightPos[i] + camPos) - fragPos);
    vec3 halfwayDir = normalize(lightRay + viewRay);
    float spec = pow(max(dot(vec3(0, 1, 0), halfwayDir), 0.0), groundShininess);
    return lightColor[i] * (spec * groundSpec);
}

vec3 spec() {
    vec3 sum = vec3(0, 0, 0);
    if (showSpecular) {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            sum += spec_light(i);
        }
    }
    return sum;
}

/* Toned down version of Magic GLSL One-liner from:
   https://blog.frost.kiwi/GLSL-noise-and-radial-gradient/ */
float noise() {
    return mix(
        -1.0 / 255.0,
        1.0 / 255.0,
        fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))))
    );
}

void main()
{
    float phi = normalize(tex_coord).y;
    float gradientHeight = (phi - lowAlt) / (highAlt - lowAlt);

    color = vec4(
        mix(
            mix(
                mix(
                    skyColor * gradientHeight + horizonColor * (1.0 - gradientHeight),
                    horizonColor,
                    float(phi < lowAlt)
                ),
                skyColor,
                float(phi > highAlt)
            ),
            groundColor + spec(),
            float(phi <= 0.0)
        ),
        1.0
    );
    
    for (int i = 0; i < MAX_LIGHTS; i++) {
        color.rgb = draw_celestial(color.rgb, phi, gradientHeight, i);
    }
    
    float dist = min(
        mix(
            -dot(camPos, vec3(0, 1, 0)) / dot(normalize(fragPos - camPos), vec3(0, 1, 0)),
            maxHazeDist,
            float(phi >= 0.0)
        ),
        maxHazeDist
    );
    color.rgb = apply_fog(color.rgb, dist);
    if (dither) color.rgb += noise();
}
