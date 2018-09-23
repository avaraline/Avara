#version 330 core

in vec3 fragmentColor;
in vec3 fragmentNormal;

uniform vec3 light0 = vec3(-0.136808, -0.282843, 0.375877);
uniform vec3 light1 = vec3(0.102606, -0.102606, -0.281908);
uniform vec3 light2 = vec3(0, 0, 0);
uniform vec3 light3 = vec3(0, 0, 0);
uniform float ambient = 0;
uniform float lights_active = 1.0;
uniform float decal;

out vec3 color;

vec3 lightColor = vec3(1, 1, 1);

vec3 diffuse_light(vec3 light) {
    vec3 dir = fragmentNormal * mix(-1.0, 1.0, gl_FrontFacing);
    return max(dot(dir, light), 0.0) * lightColor;
}

vec3 diffuse() {
    return diffuse_light(light0)
            + diffuse_light(light1)
            + diffuse_light(light2)
            + diffuse_light(light3);

}

vec3 light_color() { 
    return mix(
        ambient * lightColor * fragmentColor,
        ((ambient * lightColor) + diffuse()) * fragmentColor,
        lights_active
    );
}

void main() {
    color = light_color();
}