layout(location = 0) in vec3 vertexPosition;

uniform mat4 view;
uniform mat4 proj;
uniform vec3 camPos;

out vec3 tex_coord;
out vec3 fragPos;

void main()
{
    tex_coord = vertexPosition;
    gl_Position = proj * view * vec4(vertexPosition, 1.0);
    fragPos = vertexPosition + camPos;
}
