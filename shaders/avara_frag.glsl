#version 330 core

//in vec3 fragmentPos;
in vec3 fragmentColor;
in vec3 fragmentNormal;

uniform vec3 light0 = vec3(0, 45, 20);
uniform vec3 light1 = vec3(0, 20, 200);
uniform vec3 light2 = vec3(0, 0, 0);
uniform vec3 light3 = vec3(0, 0, 0);
uniform float ambient = 0;
uniform float lights_active = 1.0;

out vec3 color;

vec3 lightColor = vec3(1, 1, 1);

vec3 to_cartesian (in float azimuth, in float elevation)
{
    return vec3(sin(azimuth) * cos(elevation),
                -sin(elevation),
                -cos(azimuth) * cos(elevation));
}

vec3 calc_light (vec3 light) {
    vec3 lightDir = to_cartesian(radians(light.z), radians(light.y));
    //vec3 lightDir = normalize(fragmentNormal - lightPos);
    return max(dot(fragmentNormal, lightDir), 0.0) * lightColor * light.x;
}

void main()
{
    
    color = mix(
            ambient * lightColor * fragmentColor,
            ((ambient * lightColor) + (calc_light(light0) 
                                    + calc_light(light1)
                                    + calc_light(light2)
                                    + calc_light(light3))) * fragmentColor,
            lights_active);
    
    //color = fragmentNormal;
}
