#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 view;
uniform mat4 modelview;
uniform mat3 normal_transform;

out vec3 fragmentColor;
out vec3 fragmentNormal;

mat4 frustum(float angle_of_view, float aspect_ratio, float z_near, float z_far) {
    return mat4(
        vec4(1.0/tan(angle_of_view),           0.0, 0.0, 0.0),
        vec4(0.0, aspect_ratio/tan(angle_of_view),  0.0, 0.0),
        vec4(0.0, 0.0,    (z_far+z_near)/(z_far-z_near), 1.0),
        vec4(0.0, 0.0, -2.0*z_far*z_near/(z_far-z_near), 0.0)
    );
}

void main()
{
    mat4 proj = frustum(radians(30.0), 4.0/3.0, 0.5, 500.0);
    vec4 pos = vec4(vertexPosition_modelspace, 1.0);
    gl_Position = proj * (modelview * pos);// modelview * proj * view;

    fragmentColor = vertexColor;
    fragmentNormal = normal_transform * vertexNormal;
}
