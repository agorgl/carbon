#version 330
#include <mat_params>
#include <inc/tonemap>
#include <inc/lighting>

out vec4 fcolor;
in vec3 vpos;
in vec2 vtco;
in mat3 vtbn;

void main()
{
    vec4 bcolor = bcolor(vtco);
    vec3 normal = normal(vtco, vtbn);
    vec2 mtlrgn = mtlrgn(vtco);

    vec3 view = normalize(view_pos - vpos);
    vec3 ldir = normalize(light_pos);
    vec4 lcol = light_col;

    material mat;
    init_material(mat);
    mat.base_color = bcolor;
    mat.normal     = normal;
    mat.metallic   = mtlrgn.y;
    mat.roughness  = mtlrgn.x;

    light light;
    light.color       = lcol.rgb;
    light.intensity   = lcol.a;
    light.direction   = ldir;
    light.attenuation = 1.0;

    vec3 color = vec3(0.0);
    pixel_params pixel;
    get_pixel_params(mat, pixel);
    apply_directional_light(mat, pixel, light, view, color);

    color = tonemap(color);
    color = rgb_to_srgb(color);
    color = mix(vpos, color, has_bcolor_map);

    fcolor = vec4(color, 1.0);
}
