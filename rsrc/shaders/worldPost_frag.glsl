in vec2 texCoords;

uniform sampler2D screenTexture;

out vec4 color;

void main()
{
    color = texture(screenTexture, texCoords);
}
