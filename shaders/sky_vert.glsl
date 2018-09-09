#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 invMatrix;

out vec3 tilt;

void main()
{
    gl_Position = vec4(vertexPosition, 1.0) * vec4(1.0, -1.0, -1.0, 1.0);
    vec4 flipped = gl_Position * vec4(1.0, -0.4, 1.0, 1.0);
    tilt = normalize(invMatrix * flipped).xyz;
}
