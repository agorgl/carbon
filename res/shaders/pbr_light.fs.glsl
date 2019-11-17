#version 330
out vec4 fcolor;

in vec3 vpos;
in vec2 vtco;
in mat3 vtbn;

uniform float has_base_color;
uniform sampler2D base_color;

uniform float has_normal_map;
uniform sampler2D normal_map;

const vec3 light_dir = vec3(0.2, 1.0, 0.2);

void main()
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

    vec3 base_color = texture(base_color, vtco).rgb;
    vec3 ldir = normalize(light_dir);
    vec3 lcol = vec3(1.0);

    float kd = max(0.0, dot(normal, ldir));
    vec3 diffuse = kd * base_color * lcol;

    fcolor = vec4(mix(normal, diffuse, has_base_color), 1.0);
}
