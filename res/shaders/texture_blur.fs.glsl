#version 330

#include <inc/blur>

out vec4 fcolor;
in vec2 uv;

uniform sampler2D tex;
uniform float dir;

void main()
{
    uint v = uint(dir);
    vec2 d = vec2(v & 1u, (1u << v) & 1u);
    vec4 b = blur5(tex, uv, textureSize(tex, 0), d);
    fcolor = b;
}
