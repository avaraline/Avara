#version 330 core

in vec3 tex_coord;

uniform vec3 groundColor;
uniform vec3 horizonColor;
uniform vec3 skyColor;
uniform float lowAlt = 0;
uniform float highAlt = .05;

out vec4 color;

void main()
{
    float phi = normalize(tex_coord).y;
    float gradientHeight = (phi - lowAlt) / (highAlt - lowAlt);

    color = vec4(
        mix(
            mix(
                mix(
                    skyColor * gradientHeight + horizonColor * (1.0 - gradientHeight),
                    horizonColor,
                    float(phi < lowAlt)
                ),
                skyColor,
                float(phi > highAlt)
            ),
            groundColor,
            float(phi <= 0.0)
        ),
        1.0
    );
}
