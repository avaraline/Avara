#version 330 core

in vec3 tex_coord;
in vec3 camPos;

uniform vec3 groundColor;
uniform vec3 horizonColor;
uniform vec3 skyColor;
uniform float lowAlt = 0;
uniform float highAlt = .05;
uniform float hazeDensity = 0;
uniform float maxHazeDist = 180.0;

out vec4 color;

vec3 apply_fog(vec3 color, float dist)
{
    vec3 hazeColor = mix(skyColor, horizonColor, max(highAlt * 2, 1.0));
    vec3 extColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
    vec3 insColor = vec3(exp(-dist * hazeDensity), exp(-dist * hazeDensity), exp(-dist * hazeDensity));
    return color * extColor + hazeColor * (1.0 - insColor);
}

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
    
    float dist = clamp((pow(clamp(phi + 1, 0.0, 1.0), 4) * maxHazeDist) + camPos.y, 0, maxHazeDist);
    color.rgb = apply_fog(color.rgb, dist);
}
