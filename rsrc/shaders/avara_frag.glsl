#version 330 core

in vec4 fragmentColor;
in vec3 fragmentNormal;
in vec3 fragPos;

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
uniform float specularStrengh = 0.5;

out vec4 color;

vec3 diffuseLight(vec3 light, vec3 lightColor) {
    return max(dot(fragmentNormal, light), 0.0) * lightColor;
}

vec3 diffuse() {
    return diffuseLight(light0, light0Color)
            + diffuseLight(light1, light1Color)
            + diffuseLight(light2, light2Color)
            + diffuseLight(light3, light3Color);

}

vec3 specLight(vec3 light, vec3 lightColor) {
    vec3 viewDir = normalize(vec3(0,0,0) - fragPos);
    vec3 lightDir = normalize(light - fragPos);
    vec3 reflectDir = reflect(-lightDir, fragmentNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);
    return spec * lightColor;
}

vec3 spec() {
    return specLight(light0, light0Color)
            + specLight(light1, light1Color)
            + specLight(light2, light2Color)
            + specLight(light3, light3Color);
}

vec4 lightColor() {
    vec4 amb = ambient * vec4(ambientColor, 1.0);
    vec4 flatFace = amb * fragmentColor;
    vec4 litFace = vec4((ambient * ambientColor) + diffuse() + spec(), 1.0) * fragmentColor;
    return mix(flatFace, litFace, lightsActive);
}

void main() {
    color = lightColor();
}
