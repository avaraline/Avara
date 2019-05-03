#version 330 core

//in vec3 fragmentPos;
in vec3 fragmentColor;
in vec3 fragmentNormal;

uniform vec3 light0 = vec3(0.4, 45, 20);
uniform vec3 light1 = vec3(0.3, 20, 200);
uniform vec3 light2 = vec3(0, 0, 0);
uniform vec3 light3 = vec3(0, 0, 0);
uniform float ambient = 0.4;
uniform float lights_active = 1.0;

out vec3 color;

vec3 lightColor = vec3(1, 1, 1);

vec3 to_cartesian (in float azimuth, in float elevation)
{
    return vec3(1000 * sin(azimuth) * cos(elevation),
                1000 * sin(elevation),
                -1000 * cos(azimuth) * cos(elevation));
};

vec3 add_light (vec3 light, vec3 current) {
    vec3 lightPos = to_cartesian(radians(light.z), radians(light.y));
    vec3 lightDir = normalize(fragmentNormal - lightPos);
    vec3 diff = max(dot(fragmentNormal, lightDir), 0.0) * lightColor * light.x;
    return current + diff;
}

void main()
{
    color = mix(
            (ambient * lightColor * fragmentColor),
            (add_light(light0,
             add_light(light1,
             add_light(light2,
             add_light(light3, ambient * lightColor)))) * fragmentColor),
            lights_active);
}
