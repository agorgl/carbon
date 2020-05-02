#version 330 core
#include <inc/shadow>

in vec4 vpos;
out vec4 fcolor;

void main()
{
    float depth = (vpos.z / vpos.w) * 0.5 + 0.5;
    vec2 exponents = evsm_exponents(positive_exponent, negative_exponent, vec3(1.0));
    vec2 warped_depth = warp_depth(depth, exponents);
    fcolor = vec4(warped_depth, warped_depth * warped_depth);
}
