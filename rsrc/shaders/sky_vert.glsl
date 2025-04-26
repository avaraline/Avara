#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 view;
uniform mat4 invView;
uniform mat4 proj;

out vec3 tex_coord;
out vec3 camPos;
out vec3 fragPos;

void main()
{
	tex_coord = vertexPosition;
    gl_Position = proj * view * vec4(vertexPosition, 1.0);
    camPos = vec3(invView[3][0], invView[3][1], invView[3][2]);
    fragPos = vertexPosition + camPos;
}
