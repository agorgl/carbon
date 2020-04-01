#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anrm;

out vec3 vnrm;
uniform mat4 mvp;

void main()
{
    vnrm = anrm;
    gl_Position = mvp * vec4(apos, 1.0);
}
