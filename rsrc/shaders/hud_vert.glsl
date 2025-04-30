#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec4 vertexSpecular;
layout(location = 3) in vec3 vertexNormal;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec4 fragmentColor;
out vec4 fragmentSpecular;
out vec3 fragmentNormal;

void main() {
    vec4 pos = vec4(vertexPosition_modelspace, 1.0);
    gl_Position = proj * (pos * model * view);
    fragmentColor = vertexColor;
    fragmentSpecular = vertexSpecular;
    fragmentNormal = vertexNormal;
}
