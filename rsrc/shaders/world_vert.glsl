#define MAX_LIGHTS 4

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec4 vertexSpecular;
layout(location = 3) in float vertexGlow;
layout(location = 4) in float vertexReserved1;
layout(location = 5) in float vertexReserved2;
layout(location = 6) in float vertexReserved3;
layout(location = 7) in vec3 vertexNormal;

uniform vec3 lightDir[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform float ambient; // = 0.0;
uniform float extraAmbient; // = 0.0;
uniform vec3 ambientColor; // = vec3(1, 1, 1);
uniform bool lightsActive; // = true;
uniform float maxShininess;
uniform float maxGlow;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normalTransform;

out vec4 fragmentColor;
out vec3 fragmentSpecular;
out float fragmentShininess;
out float fragmentGlow;
out vec3 fragmentNormal;
out vec3 fragPos;
out vec3 baseLightColor;

vec3 diffuse_light(int i, vec3 normal) {
    return max(dot(normal, lightDir[i]), 0.0) * lightColor[i];
}

vec3 diffuse(vec3 normal) {
    vec3 sum = vec3(0, 0, 0);
    if (lightsActive) {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            sum += diffuse_light(i, normal);
        }
    }
    return sum;

}

void main() {
    vec4 pos = vec4(vertexPosition_modelspace, 1.0);
    gl_Position = proj * (pos * model * view);
    fragmentColor = vertexColor;
    fragmentSpecular = vertexSpecular.rgb;
    fragmentShininess = vertexSpecular.a * maxShininess;
    fragmentGlow = vertexGlow * maxGlow;
    fragmentNormal = vertexNormal * normalTransform;
    fragPos = (pos * model).xyz;
    baseLightColor = ((ambient + extraAmbient) * ambientColor) + diffuse(fragmentNormal);
}
