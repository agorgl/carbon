#version 330 core
layout (location = 0) in vec3 apos;

out vec4 vpos;

uniform mat4 modl;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    vpos = proj * view * modl * vec4(apos, 1.0);
    gl_Position = vpos;
}
