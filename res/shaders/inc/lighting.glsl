//
// lighting.glsl
//
#ifndef _LIGHTING_GLSL_
#define _LIGHTING_GLSL_

#include <material>
#include <brdf>

struct light {
    vec3 color;
    float intensity; // pre-exposed intensity
    vec3 direction;
    float attenuation;
};

struct pixel_params {
    vec3  diffuse_color;
    float perceptual_roughness;
    vec3  f0;
    float roughness;
    vec3  energy_compensation;
};

float compute_micro_shadowing(float NoL, float visibility)
{
    // Chan 2018, "Material Advances in Call of Duty: WWII"
    float aperture = inversesqrt(1.0 - visibility);
    float micro_shadow = saturate(NoL * aperture);
    return micro_shadow * micro_shadow;
}

//------------------------------------------------------------------------------
// Shading Model Standard
//------------------------------------------------------------------------------
vec3 isotropic_lobe(const pixel_params pixel, const light light, const vec3 h,
                    float NoV, float NoL, float NoH, float LoH)
{
    float D = distribution(pixel.roughness, NoH, h);
    float V = visibility(pixel.roughness, NoV, NoL);
    vec3  F = fresnel(pixel.f0, LoH);
    return (D * V) * F;
}

vec3 specular_lobe(const pixel_params pixel, const light light, const vec3 h,
                   float NoV, float NoL, float NoH, float LoH)
{
    return isotropic_lobe(pixel, light, h, NoV, NoL, NoH, LoH);
}

vec3 diffuse_lobe(const pixel_params pixel, float NoV, float NoL, float LoH)
{
    return pixel.diffuse_color * diffuse(pixel.roughness, NoV, NoL, LoH);
}

/**
 * Evaluates lit materials with the standard shading model. This model comprises
 * of 2 BRDFs: an optional clear coat BRDF, and a regular surface BRDF.
 *
 * Surface BRDF
 * The surface BRDF uses a diffuse lobe and a specular lobe to render both
 * dielectrics and conductors. The specular lobe is based on the Cook-Torrance
 * micro-facet model (see brdf for more details). The specular is isotropic.
 */
vec3 surface_shading(const pixel_params pixel, const light light, vec3 normal, vec3 view, float occlusion)
{
    vec3 h = normalize(view + light.direction);
    float NoL = saturate(dot(normal, light.direction));
    float NoH = saturate(dot(normal, h));
    float LoH = saturate(dot(light.direction, h));
    float NoV = clamp_NoV(dot(normal, view));

    vec3 Fr = specular_lobe(pixel, light, h, NoV, NoL, NoH, LoH);
    vec3 Fd = diffuse_lobe(pixel, NoV, NoL, LoH);

    // TODO: attenuate the diffuse lobe to avoid energy gain
    // The energy compensation term is used to counteract the darkening effect at high roughness
    vec3 color = Fd + Fr * pixel.energy_compensation;
    return (color * light.color)
         * (light.intensity * light.attenuation * NoL * occlusion);
}

//------------------------------------------------------------------------------
// Lighting
//------------------------------------------------------------------------------
float compute_masked_alpha(float a)
{
    // Use derivatives to smooth alpha tested edges
    return (a - 0.1) / max(fwidth(a), 1e-3) + 0.5;
}

void apply_alpha_mask(inout vec4 base_color)
{
    base_color.a = compute_masked_alpha(base_color.a);
    if (base_color.a <= 0.0)
        discard;
}

void get_common_pixel_params(const material material, inout pixel_params pixel)
{
    vec4 base_color = material.base_color;
    float reflectance = compute_dielectric_f0(material.reflectance);
    apply_alpha_mask(base_color);
    pixel.diffuse_color = compute_diffuse_color(base_color, material.metallic);
    pixel.f0 = compute_f0(base_color, material.metallic, reflectance);
}

void get_roughness_pixel_params(const material material, inout pixel_params pixel)
{
    float perceptual_roughness = material.roughness;
    // Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    perceptual_roughness = clamp(perceptual_roughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.perceptual_roughness = perceptual_roughness;
    pixel.roughness = perceptual_roughness_to_roughness(perceptual_roughness);
}

void get_energy_compensation_pixel_params(inout pixel_params pixel)
{
    // Energy compensation for multiple scattering in a microfacet model
    // See "Multiple-Scattering Microfacet BSDFs with the Smith Model"
    //pixel.energy_compensation = 1.0 + pixel.f0 * (1.0 / pixel.dfg.y - 1.0);
    pixel.energy_compensation = vec3(1.0);
}

/**
 * Computes all the parameters required to shade the current pixel/fragment.
 * These parameters are derived from the material structure computed
 * by the user's material code.
 *
 * This function is also responsible for discarding the fragment when alpha
 * testing fails.
 */
void get_pixel_params(const material material, out pixel_params pixel)
{
    get_common_pixel_params(material, pixel);
    get_roughness_pixel_params(material, pixel);
    get_energy_compensation_pixel_params(pixel);
}

void apply_directional_light(const material material, const pixel_params pixel, light light, vec3 view, inout vec3 color)
{
    float NoL = saturate(dot(material.normal, light.direction));
    float visibility = 1.0;
    if (NoL < 0.0)
        return;
    color += surface_shading(pixel, light, material.normal, view, visibility);
}

#endif
