#version 330 core

//in vec3 fragmentPos;
in vec3 fragmentColor;
in vec3 fragmentNormal;

out vec3 color;

void main()
{
    vec3 lightColor = vec3(1, 1, 1);
    vec3 lightPos = vec3(0, 1, 0.6);

    vec3 ambient = 0.2 * lightColor;

    vec3 lightDir = normalize(fragmentNormal - lightPos);
    float diff = max(dot(fragmentNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    color = (ambient + diffuse) * fragmentColor;
}
