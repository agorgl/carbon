//
// material.glsl
//
#ifndef _MATERIAL_GLSL_
#define _MATERIAL_GLSL_

#include <math>

#define MIN_PERCEPTUAL_ROUGHNESS 0.045
#define MIN_ROUGHNESS            0.002025
#define MIN_N_DOT_V              1e-4

struct material {
    vec4  base_color;
    float roughness;
    float metallic;
    float reflectance;
    float ambient_occlusion;
    vec4  emissive;
    vec3  normal;
};

void init_material(out material mat)
{
    mat.base_color        = vec4(1.0);
    mat.roughness         = 1.0;
    mat.metallic          = 0.0;
    mat.reflectance       = 0.0;
    mat.ambient_occlusion = 1.0;
    mat.emissive          = vec4(0.0);
    mat.normal            = vec3(0.0);
}

float clamp_NoV(float NoV)
{
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return max(NoV, MIN_N_DOT_V);
}

vec3 compute_diffuse_color(const vec4 base_color, float metallic)
{
    return base_color.rgb * (1.0 - metallic);
}

vec3 compute_f0(const vec4 base_color, float metallic, float reflectance)
{
    return base_color.rgb * metallic + (reflectance * (1.0 - metallic));
}

float compute_dielectric_f0(float reflectance)
{
    return 0.16 * reflectance * reflectance;
}

float compute_metallic_from_specular_color(const vec3 specular_color)
{
    return max3(specular_color);
}

float compute_roughness_from_glossiness(float glossiness)
{
    return 1.0 - glossiness;
}

float perceptual_roughness_to_roughness(float perceptual_roughness)
{
    return perceptual_roughness * perceptual_roughness;
}

float roughness_to_perceptual_roughness(float roughness)
{
    return sqrt(roughness);
}

float ior_to_f0(float transmitted_ior, float incident_ior)
{
    return sq((transmitted_ior - incident_ior) / (transmitted_ior + incident_ior));
}

float f0_to_ior(float f0)
{
    float r = sqrt(f0);
    return (1.0 + r) / (1.0 - r);
}

#endif
