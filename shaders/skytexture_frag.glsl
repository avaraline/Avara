#version 330 core

#ifdef GL_ES
precision mediump float;
#endif

in vec3 tex_coord;

uniform vec3 groundColor;

out vec4 color;

uniform samplerCube skybox;

void main()
{
    float phi = normalize(tex_coord).y;

    if(phi > 0.0)
        color = texture(skybox, tex_coord);
    else
        color = vec4(groundColor, 1.0);
}






