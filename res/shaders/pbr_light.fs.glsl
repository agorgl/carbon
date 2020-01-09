#version 330
#include <inc/color>
out vec4 fcolor;

in vec3 vpos;
in vec2 vtco;
in mat3 vtbn;

uniform vec4 bcolor_val;
uniform vec2 mtlrgn_val;

uniform float has_bcolor_map;
uniform sampler2D bcolor_map;
uniform float has_normal_map;
uniform sampler2D normal_map;
uniform float has_mtlrgn_map;
uniform sampler2D mtlrgn_map;

uniform vec3 view_pos;
const vec3 light_dir = vec3(0.2, 1.0, 0.2);

vec4 bcolor()
{
    vec4 tex_bcolor = texture(bcolor_map, vtco);
    tex_bcolor = vec4(srgb_to_rgb(tex_bcolor.rgb), tex_bcolor.a);
    return mix(vec4(1.0), tex_bcolor, has_bcolor_map);
}

vec3 normal()
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

vec2 mtlrgn()
{
    vec2 tex_mtlrgn = texture(mtlrgn_map, vtco).gb;
    return mix(vec2(1.0), tex_mtlrgn, has_mtlrgn_map);
}

void main()
{
    // Material attributes
    vec4 bcolor = bcolor();
    vec3 normal = normal();
    vec2 mtlrgn = mtlrgn();

    // Light params
    vec3 view = normalize(view_pos - vpos);
    vec3 ldir = normalize(light_dir);
    vec3 lcol = vec3(1.0);

    float kd = max(0.0, dot(normal, ldir));
    vec3 diffuse = kd * bcolor.rgb * lcol;

    fcolor = vec4(mix(normal, diffuse, has_bcolor_map), 1.0);
}
