#version 330

out vec4 fcolor;
in vec3 vnrm;

uniform samplerCube probe;

void main()
{
    vec3 color = texture(probe, vnrm).rgb;
    fcolor = vec4(color, 1.0);
}
