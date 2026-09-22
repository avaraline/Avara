layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec4 vertexSpecular;
layout(location = 3) in float vertexGlow;
layout(location = 4) in float vertexReserved1;
layout(location = 5) in float vertexReserved2;
layout(location = 6) in float vertexReserved3;
layout(location = 7) in vec3 vertexNormal;

uniform float ambient; // = 0.0;
uniform float extraAmbient;
uniform bool lightsActive; // = true;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec4 fragmentColor;
out float fragmentGlow;
out vec3 fragmentNormal;
out vec3 baseLightColor;

vec3 light0 = vec3(0.136808053, 0.282842726, -0.375877053);
vec3 light1 = vec3(-0.102606051, 0.102606043, 0.281907797);
vec3 lightColor = vec3(1, 1, 1);

vec3 diffuse_light(vec3 light, vec3 normal) {
    return max(dot(normal, light), 0.0) * lightColor;
}

vec3 diffuse(vec3 normal) {
    vec3 sum = vec3(0, 0, 0);
    if (lightsActive) {
        sum += diffuse_light(light0, normal);
        sum += diffuse_light(light1, normal);
    }
    return sum;
}

void main() {
    vec4 pos = vec4(vertexPosition_modelspace, 1.0);
    gl_Position = proj * (pos * model * view);
    fragmentColor = vertexColor;
    fragmentGlow = vertexGlow;
    fragmentNormal = vertexNormal;
    baseLightColor = ((ambient + extraAmbient) * lightColor) + diffuse(fragmentNormal);
}
