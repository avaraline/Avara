#version 330 core

in vec2 texCoords;

uniform float hudAlpha = 1.0;
uniform sampler2D screenTexture;

out vec4 color;

void main()
{
    color = texture(screenTexture, texCoords);
    color[3] *= hudAlpha;
}
