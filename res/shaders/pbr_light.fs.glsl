#version 330
#include <mat_params>
#include <inc/tonemap>

out vec4 fcolor;
in vec3 vpos;
in vec2 vtco;
in mat3 vtbn;

const vec3 light_dir = vec3(0.2, 1.0, 0.2);

void main()
{
    // Material attributes
    vec4 bcolor = bcolor(vtco);
    vec3 normal = normal(vtco, vtbn);
    vec2 mtlrgn = mtlrgn(vtco);

    // Light params
    vec3 view = normalize(view_pos - vpos);
    vec3 ldir = normalize(light_dir);
    vec3 lcol = vec3(1.0);

    float kd = max(0.0, dot(normal, ldir));
    vec3 diffuse = kd * bcolor.rgb * lcol;

    fcolor = vec4(mix(normal, diffuse, has_bcolor_map), 1.0);
}
