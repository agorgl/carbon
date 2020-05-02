#include <inc/common>
#include <inc/color>
#include <inc/shadow>

uniform vec3 view_pos;

uniform vec3 light_pos;
uniform vec4 light_col;

uniform vec4 bcolor_val;
uniform vec2 mtlrgn_val;

uniform float has_bcolor_map;
uniform sampler2D bcolor_map;
uniform float has_normal_map;
uniform sampler2D normal_map;
uniform float has_mtlrgn_map;
uniform sampler2D mtlrgn_map;

uniform sampler2D shadow_map;
uniform mat4 lightsp_mat;

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

bool is_vertex_in_shadow_map(vec3 coord)
{
    return coord.z > 0.0
        && coord.x > 0.0
        && coord.y > 0.0
        && coord.x <= 1.0
        && coord.y <= 1.0;
}

float shadow(vec3 vpos)
{
    // Vertex in light space
    vec4 ls_pos = lightsp_mat * vec4(vpos, 1.0);
    // Perform perspective divide
    vec3 proj_coords = (ls_pos.xyz / ls_pos.w);
    // Transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;
    // Check if vertex is captured by the shadow map
    if (!is_vertex_in_shadow_map(proj_coords))
        return 1.0;
    // Calculate shadow coefficient
    float shadow = shadow_visibility(shadow_map, proj_coords);
    return shadow;
}
