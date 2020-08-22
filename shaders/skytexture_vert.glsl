#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 view;
uniform mat4 proj;

out vec3 tex_coord;

void main()
{
	tex_coord = vertexPosition;
    //gl_Position = proj * view * vec4(vertexPosition, 1.0);
    vec4 pos = proj * view * vec4(vertexPosition, 1.0);
    gl_Position = pos.xyww;

}
