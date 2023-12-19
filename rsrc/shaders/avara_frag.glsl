#version 330 core

in vec4 fragmentColor;
in vec3 fragmentNormal;
in vec3 fragPos;
in vec3 camPos;
in mat4 modelView;

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
uniform vec4 lightsActive = vec4(0, 0, 0, 0);
uniform float specularStrengh = 0.5;


out vec4 color;

vec3 diffuseLight(int index, vec3 lightPos, vec3 lightColor) {
    return mix(vec3(0), max(dot(fragmentNormal, lightPos), 0.0) * lightColor, lightsActive[index]);
}

vec3 diffuse() {
    return diffuseLight(0, light0, light0Color)
            + diffuseLight(1, light1, light1Color)
            + diffuseLight(2, light2, light2Color)
            + diffuseLight(3, light3, light3Color);

}

vec3 specLight(int index, vec3 light, vec3 lightColor) {
    vec3 viewDir = normalize(camPos - fragPos);
    vec4 lightPos = vec4(light * 1000, 1) * modelView;
    vec3 lightDir = normalize(lightPos.xyz - fragPos);
    vec3 reflectDir = reflect(-lightDir, fragmentNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);
    return mix(vec3(0), specularStrengh * spec * lightColor, lightsActive[index]);
}

vec3 spec() {
    return specLight(0, light0, light0Color);/*
            + specLight(1, light1, light1Color)
            + specLight(2, light2, light2Color)
            + specLight(3, light3, light3Color);*/
}

vec4 lightColor() {
    vec4 amb = ambient * vec4(ambientColor, 1.0);
    vec4 flatFace = amb * fragmentColor;
    vec4 lights = vec4(diffuse() + spec(), 1.0);
    vec4 litFace = (amb + lights) * fragmentColor;
    float anyLights = max(lightsActive[0], max(lightsActive[1], max(lightsActive[2], lightsActive[3])));
    return mix(flatFace, litFace, anyLights);
}

void main() {
    color = lightColor();
}
