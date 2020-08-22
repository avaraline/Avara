#version 330 core

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform float u_time;

in vec3 tex_coord;

uniform vec3 groundColor;
uniform vec3 horizonColor;
uniform vec3 skyColor;
uniform float lowAlt = 0;
uniform float highAlt = .05;

out vec4 color;

uniform samplerCube skybox;

vec3 colorA = vec3(0.149,0.141,0.912);
vec3 colorB = vec3(0.981,1.000,0.445);

void main()
{
    float phi = normalize(tex_coord).y;

    if(tex_coord.y > 0.0) {
        //color = vec4(.5, .5, .8, 1.0);
        color = texture(skybox, tex_coord);
    }
    else
        color = vec4(mix(
                mix( // TODO: lowAlt messes up the gradient
                    mix(skyColor * (phi / highAlt) + horizonColor * (1.0 - (phi / highAlt)),
                        horizonColor, float(phi < lowAlt)
                    ), skyColor, float(phi > highAlt)
                ), groundColor, float(phi <= 0.0)
            ), 1.0);
    
 //   vec3 mixcolor = vec3(0.0);
    
 //   float pct = abs(sin(u_time));
    
    // Mix uses pct (a value from 0-1) to
    // mix the two colors
 //   mixcolor = mix(colorA, colorB, pct);
    
 //   color = vec4(mixcolor,1.0);
}






