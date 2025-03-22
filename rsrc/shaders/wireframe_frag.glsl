#version 330 core

in vec4 fragmentColor;

out vec4 color;

void main() {
    color = vec4(1.0 - fragmentColor.r, 1.0 - fragmentColor.g, 1.0 - fragmentColor.b, 1);
}
