in vec4 fragmentColor;
in float fragmentGlow;
in vec3 fragmentNormal;
in vec3 baseLightColor;

uniform float ambient; // = 0.0;
uniform float extraAmbient; // = 0.0;
uniform bool lightsActive; // = true;

out vec4 color;

vec4 light_color() {
    return mix(
        vec4(baseLightColor, 1.0) * fragmentColor,
        (1 + extraAmbient) * fragmentColor,
        float(fragmentGlow > 0.0)
    );
}

void main() {
    color = light_color();
}
