#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 proj;

void main()
{
    TexCoords = aPos;
    // Strip translation from view matrix so skybox stays centered on camera
    mat4 staticView = mat4(mat3(view));
    vec4 pos = proj * staticView * vec4(aPos, 1.0);
    // Setting z = w places the skybox at the far clipping plane (depth = 1.0)
    gl_Position = pos.xyww;
}
