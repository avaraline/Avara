#version 330 core

in vec3 tex_coord;

uniform vec3 groundColor;
uniform vec3 horizonColor;
uniform vec3 skyColor;
uniform float lowAlt = 0;
uniform float highAlt = .05;

out vec3 color;

void main()
{
    float phi = normalize(tex_coord).y;

    color = mix(
                mix( // TODO: lowAlt messes up the gradient
                    mix(skyColor * (phi / highAlt) + horizonColor * (1.0 - (phi / highAlt)),
                        horizonColor, float(phi < lowAlt)
                    ), skyColor, float(phi > highAlt)
                ), groundColor, float(phi <= 0.0)
            );
}
