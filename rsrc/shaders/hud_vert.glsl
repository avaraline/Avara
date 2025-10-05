layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec4 vertexSpecular;
layout(location = 3) in float vertexGlow;
layout(location = 4) in float vertexReserved1;
layout(location = 5) in float vertexReserved2;
layout(location = 6) in float vertexReserved3;
layout(location = 7) in vec3 vertexNormal;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec4 fragmentColor;
out vec4 fragmentSpecular;
out float fragmentGlow;
out vec3 fragmentNormal;

void main() {
    vec4 pos = vec4(vertexPosition_modelspace, 1.0);
    gl_Position = proj * (pos * model * view);
    fragmentColor = vertexColor;
    fragmentSpecular = vertexSpecular;
    fragmentGlow = vertexGlow;
    fragmentNormal = vertexNormal;
}
