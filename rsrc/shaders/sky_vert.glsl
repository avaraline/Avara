#version 120
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 vertexPosition;

uniform mat4 view;
uniform mat4 proj;

out vec3 tex_coord;

void main()
{
	tex_coord = vertexPosition;
    gl_Position = proj * view * vec4(vertexPosition, 1.0);
}
