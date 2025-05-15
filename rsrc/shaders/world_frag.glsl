#version 330 core
#define MAX_LIGHTS 4

in vec4 gl_FragCoord;
in vec4 fragmentColor;
in vec4 fragmentSpecular;
in vec3 fragmentNormal;
in vec3 fragPos;

uniform vec3 camPos;
uniform vec3 lightDir[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightPos[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightColor[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1));
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
    vec3 extColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
    vec3 insColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
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

vec3 spec_light(int i) {
    if (!lightApplySpecular[i] || fragmentSpecular.a == 0 || !lightsActive) return vec3(0);
    vec3 viewRay = normalize(camPos - fragPos);
    vec3 lightRay = normalize((lightPos[i] + camPos) - fragPos);
    vec3 halfwayDir = normalize(lightRay + viewRay);
    float spec = pow(max(dot(fragmentNormal, -halfwayDir), 0.0), fragmentSpecular.a * 1024.0);
    return lightColor[i] * (spec * fragmentSpecular.rgb);
}

vec3 spec() {
    vec3 sum = vec3(0, 0, 0);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        sum += spec_light(i);
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

vec4 light_color() {
    return mix(
        ambient * vec4(ambientColor, 1.0) * fragmentColor,
        vec4((ambient * ambientColor) + diffuse() + spec(), 1.0) * fragmentColor,
        float(lightsActive)
    );
}

void main() {
    color = light_color();
    
    float dist = length(fragPos - camPos);
    color.rgb = apply_fog(color.rgb, dist);
    
    float yonFadeRange = min(5.0, objectYon - (objectYon * 0.9));
    float yonFadeDist = objectYon - yonFadeRange;
    float alphaMult = pow(clamp((yonFadeRange + yonFadeDist - dist) / yonFadeRange, 0.0, 1.0), 0.5);
    color.a *= alphaMult;
    color.rgb += noise();
}
