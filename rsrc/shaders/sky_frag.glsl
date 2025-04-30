#version 330 core
#define MAX_LIGHTS 4

in vec3 tex_coord;
in vec3 camPos;
in vec3 fragPos;

uniform vec3 lights[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightColors[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1));
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

vec3 apply_fog(vec3 color, float dist)
{
    vec3 hazeColor = mix(skyColor, horizonColor, max(highAlt * 2, 1.0));
    vec3 extColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
    vec3 insColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
    return color * extColor + hazeColor * (1.0 - insColor);
}

vec3 spec_light(int i) {
    if (!lightApplySpecular[i] || groundShininess == 0) return vec3(0);
    vec3 viewDir = normalize(camPos - fragPos);
    vec4 lightPos = vec4(lights[i] * -1000, 1);
    vec3 lightDir = normalize(lightPos.xyz - fragPos);
    vec3 reflectDir = reflect(-lightDir, vec3(0, 1, 0));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), groundShininess * 2048.0);
    return lightColors[i] * (spec * groundSpec);
}

vec3 spec() {
    vec3 sum = vec3(0, 0, 0);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        sum += spec_light(i);
    }
    return sum;
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
    
    float dist = min(
        mix(
            -dot(camPos, vec3(0, 1, 0)) / dot(normalize(fragPos - camPos), vec3(0, 1, 0)),
            maxHazeDist,
            float(phi >= 0.0)
        ),
        maxHazeDist
    );
    color.rgb = apply_fog(color.rgb, dist);
}
