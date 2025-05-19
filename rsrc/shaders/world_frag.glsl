#version 330 core
#define MAX_LIGHTS 4

in vec4 gl_FragCoord;
in vec4 fragmentColor;
in vec3 fragmentSpecular;
in float fragmentShininess;
in vec3 fragmentNormal;
in vec3 fragPos;

uniform vec3 camPos;
uniform bool dither;
uniform bool showSpecular;
uniform vec3 lightDir[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightPos[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 adjustedLightPos[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightColor[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1));
uniform float lightCelestialRadius[MAX_LIGHTS] = float[MAX_LIGHTS](0.0, 0.0, 0.0, 0.0);
uniform bool lightApplySpecular[MAX_LIGHTS] = bool[MAX_LIGHTS](false, false, false, false);
uniform float ambient = 0.0;
uniform vec3 ambientColor = vec3(1, 1, 1);
uniform bool lightsActive = true;
uniform float worldYon = 180.0;
uniform float objectYon = 180.0;
uniform vec3 skyColor;
uniform vec3 horizonColor;
uniform float highAlt;
uniform float hazeDensity;

out vec4 color;

vec3 apply_fog(vec3 color, float dist)
{
    vec3 hazeColor = mix(skyColor, horizonColor, max(highAlt * 2, 1.0));
    float coefficient = exp(-dist * hazeDensity);
    vec3 extColor = vec3(coefficient, coefficient, coefficient);
    vec3 insColor = vec3(coefficient, coefficient, coefficient);
    return color * extColor + hazeColor * (1.0 - insColor);
}

vec3 diffuse_light(int i) {
    return max(dot(fragmentNormal, lightDir[i]), 0.0) * lightColor[i];
}

vec3 diffuse() {
    vec3 sum = vec3(0, 0, 0);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        sum += diffuse_light(i);
    }
    return sum;

}

vec3 spec_light(int i, vec3 viewDir) {
    if (!lightApplySpecular[i] || fragmentShininess == 0 || !lightsActive) return vec3(0);
    vec3 lightDir = normalize(adjustedLightPos[i] - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(fragmentNormal, -halfwayDir), 0.0), fragmentShininess);
    return lightColor[i] * (spec * fragmentSpecular);
}

vec3 spec(vec3 viewDir) {
    vec3 sum = vec3(0, 0, 0);
    if (showSpecular) {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            sum += spec_light(i, viewDir);
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

vec4 light_color(vec3 viewDir) {
    return mix(
        ambient * vec4(ambientColor, 1.0) * fragmentColor,
        vec4((ambient * ambientColor) + diffuse() + spec(viewDir), 1.0) * fragmentColor,
        float(lightsActive)
    );
}

void main() {
    vec3 viewRay = camPos - fragPos;
    vec3 viewRayNormalized = normalize(viewRay);
    color = light_color(viewRayNormalized);
    
    float dist = length(viewRay);
    color.rgb = apply_fog(color.rgb, dist);
    
    float yonFadeRange = min(5.0, objectYon - (objectYon * 0.9));
    float yonFadeDist = objectYon - yonFadeRange;
    float alphaMult = pow(clamp((yonFadeRange + yonFadeDist - dist) / yonFadeRange, 0.0, 1.0), 0.5);
    color.a *= alphaMult;
    if (dither) color.rgb += noise();
}
