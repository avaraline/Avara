#version 330 core

in vec4 gl_FragCoord;
in vec4 fragmentColor;
in vec3 fragmentNormal;
in vec3 fragPos;
in vec3 camPos;

uniform vec3 light0 = vec3(0, 0, 0);
uniform vec3 light0Color = vec3(1, 1, 1);
uniform vec3 light1 = vec3(0, 0, 0);
uniform vec3 light1Color = vec3(1, 1, 1);
uniform vec3 light2 = vec3(0, 0, 0);
uniform vec3 light2Color = vec3(1, 1, 1);
uniform vec3 light3 = vec3(0, 0, 0);
uniform vec3 light3Color = vec3(1, 1, 1);
uniform float ambient = 0.0;
uniform vec3 ambientColor = vec3(1, 1, 1);
uniform float lightsActive = 1.0;
uniform float worldYon = 180.0;
uniform float objectYon = 180.0;
uniform vec3 horizonColor;
uniform vec3 skyColor;
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

vec3 diffuse_light(vec3 light, vec3 lightColor) {
    return max(dot(fragmentNormal, light), 0.0) * lightColor;
}

vec3 diffuse() {
    return diffuse_light(light0, light0Color)
            + diffuse_light(light1, light1Color)
            + diffuse_light(light2, light2Color)
            + diffuse_light(light3, light3Color);

}

vec4 light_color() {
    return mix(
        ambient * vec4(ambientColor, 1.0) * fragmentColor,
        vec4((ambient * ambientColor) + diffuse(), 1.0) * fragmentColor,
        lightsActive
    );
}

void main() {
    color = light_color();
    
    vec3 fragRay = camPos - fragPos;
    float dist = length(fragRay);
    color.rgb = apply_fog(color.rgb, dist);
    
    float yonFadeRange = min(5.0, objectYon - (objectYon * 0.9));
    float yonFadeDist = objectYon - yonFadeRange;
    float alphaMult = pow(clamp((yonFadeRange + yonFadeDist - dist) / yonFadeRange, 0.0, 1.0), 0.5);
    color.a *= alphaMult;
}
