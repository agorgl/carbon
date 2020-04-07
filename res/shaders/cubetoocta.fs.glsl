#version 330

#include <inc/octahedral>

out vec4 fcolor;
in vec2 uv;

uniform samplerCube probe;

void main()
{
    vec3 n = oct_decode(uv * 2.0 - 1.0);
    vec3 c = texture(probe, n).rgb;
    fcolor = vec4(c, 1.0);
}
