#version 330 core
#define MAX_LIGHTS 4

in vec4 gl_FragCoord;
in vec4 fragmentColor;
in vec4 fragmentSpecular;
in vec3 fragmentNormal;
in vec3 fragPos;
in vec3 camPos;

uniform vec3 lights[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0));
uniform vec3 lightColors[MAX_LIGHTS] = vec3[MAX_LIGHTS](vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1), vec3(1, 1, 1));
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
    return max(dot(fragmentNormal, lights[i]), 0.0) * lightColors[i];
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
    vec3 viewDir = normalize(camPos - fragPos);
    vec4 lightPos = vec4(lights[i] * -1000, 1);
    vec3 lightDir = normalize(lightPos.xyz - fragPos);
//    vec3 reflectDir = reflect(-lightDir, fragmentNormal);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), fragmentSpecular.a * 2048.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(fragmentNormal, -halfwayDir), 0.0), fragmentSpecular.a * 2048.0);
    return lightColors[i] * (spec * fragmentSpecular.rgb);
}

vec3 spec() {
    vec3 sum = vec3(0, 0, 0);
    for (int i = 0; i < MAX_LIGHTS; i++) {
        sum += spec_light(i);
    }
    return sum;
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
    
    vec3 fragRay = fragPos - camPos;
    float dist = length(fragRay);
    color.rgb = apply_fog(color.rgb, dist);
    
    float yonFadeRange = min(5.0, objectYon - (objectYon * 0.9));
    float yonFadeDist = objectYon - yonFadeRange;
    float alphaMult = pow(clamp((yonFadeRange + yonFadeDist - dist) / yonFadeRange, 0.0, 1.0), 0.5);
    color.a *= alphaMult;
}
