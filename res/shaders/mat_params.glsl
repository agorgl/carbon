#include <inc/color>

uniform vec3 view_pos;
uniform vec4 bcolor_val;
uniform vec2 mtlrgn_val;

uniform float has_bcolor_map;
uniform sampler2D bcolor_map;
uniform float has_normal_map;
uniform sampler2D normal_map;
uniform float has_mtlrgn_map;
uniform sampler2D mtlrgn_map;

vec4 bcolor(vec2 vtco)
{
    vec4 tex_bcolor = texture(bcolor_map, vtco);
    tex_bcolor = vec4(srgb_to_rgb(tex_bcolor.rgb), tex_bcolor.a);
    return mix(vec4(1.0), tex_bcolor, has_bcolor_map);
}

vec3 normal(vec2 vtco, mat3 vtbn)
{
    vec3 normal;
    bool has_nm = bool(has_normal_map);
    if (has_nm) {
        normal = texture(normal_map, vtco).rgb;
        normal = normal * 2.0 - 1.0;
        normal = vtbn * normal;
    } else {
        normal = vtbn[2];
    }
    normal = normalize(normal);
    return normal;
}

vec2 mtlrgn(vec2 vtco)
{
    vec2 tex_mtlrgn = texture(mtlrgn_map, vtco).gb;
    return mix(vec2(1.0), tex_mtlrgn, has_mtlrgn_map);
}
