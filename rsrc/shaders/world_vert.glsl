#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec4 vertexSpecular;
layout(location = 3) in vec3 vertexNormal;

uniform float maxShininess;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normalTransform;

out vec4 fragmentColor;
out vec3 fragmentSpecular;
out float fragmentShininess;
out vec3 fragmentNormal;
out vec3 fragPos;

void main() {
    vec4 pos = vec4(vertexPosition_modelspace, 1.0);
    gl_Position = proj * (pos * model * view);
    fragmentColor = vertexColor;
    fragmentSpecular = vertexSpecular.rgb;
    fragmentShininess = vertexSpecular.a * maxShininess;
    fragmentNormal = vertexNormal * normalTransform;
    fragPos = (pos * model).xyz;
}
