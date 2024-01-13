#version 330 core

in vec4 gl_FragCoord;
in vec4 fragmentColor;
in vec3 fragmentNormal;

uniform float ambient = 0.0;
uniform float hudAlpha = 1.0;
uniform float lightsActive = 1.0;

out vec4 color;

vec3 light0 = vec3(-0.136808053, -0.282842726, 0.375877053);
vec3 light1 = vec3(0.102606051, -0.102606043, -0.281907797);
vec3 lightColor = vec3(1, 1, 1);

vec3 diffuse_light(vec3 light) {
    return max(dot(fragmentNormal, light), 0.0) * lightColor;
}

vec3 diffuse() {
    return diffuse_light(light0)
            + diffuse_light(light1);

}

vec4 light_color() {
    return mix(
        ambient * vec4(lightColor, 1.0) * fragmentColor,
        vec4((ambient * lightColor) + diffuse(), 1.0) * fragmentColor,
        lightsActive
    );
}

void main() {
    color = light_color();
    color[0] = color[0] * hudAlpha * ((int(gl_FragCoord.y) % 4) / 2);
    color[1] = color[1] * hudAlpha * ((int(gl_FragCoord.y) % 4) / 2);
    color[2] = color[2] * hudAlpha * ((int(gl_FragCoord.y) % 4) / 2);
    color[3] = color[3] * hudAlpha * ((int(gl_FragCoord.y) % 4) / 2);
}

