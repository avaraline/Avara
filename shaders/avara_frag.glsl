#version 330 core

in vec3 fragmentColor;
in vec3 fragmentNormal;

uniform vec3 light0 = vec3(0, 45, 20);
uniform vec3 light1 = vec3(0, 20, 200);
uniform vec3 light2 = vec3(0, 0, 0);
uniform vec3 light3 = vec3(0, 0, 0);
uniform float ambient = 0;
uniform float lights_active = 1.0;
uniform float decal;

out vec3 color;

vec3 lightColor = vec3(1, 1, 1);

vec3 to_cartesian (vec3 light) {
    float x = cos(radians(light.y)) * light.x;
    float y = sin(radians(-light.y)) * light.x;
    float z = cos(radians(light.z)) * x;
    x = sin(radians(-light.z)) * x;
    return vec3(x, y, z);
}

vec3 calc_light (vec3 light) {
    vec3 lightPos = to_cartesian(light);
    vec3 dir = fragmentNormal * mix(-1.0, 1.0, gl_FrontFacing);
    return max(dot(dir, lightPos), 0.0) * lightColor;
}

vec3 light_color() {
    vec3 totalLight = calc_light(light0)
                    + calc_light(light1)
                    + calc_light(light2)
                    + calc_light(light3);
    return mix(
        ambient * lightColor * fragmentColor,
        ((ambient * lightColor) + totalLight) * fragmentColor,
        lights_active
    );
}

void main() {
    color = light_color();
}