#version 330 core

in vec4 fragmentColor;
in vec3 fragmentNormal;

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
uniform float lights_active = 1.0;

out vec4 color;

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
        lights_active
    );
}

void main() {
    color = light_color();
}
