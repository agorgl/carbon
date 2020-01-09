//
// brdf.glsl
//
#ifndef _BRDF_GLSL_
#define _BRDF_GLSL_

#include <math>

//------------------------------------------------------------------------------
// Enumerations
//------------------------------------------------------------------------------
// Diffuse BRDFs
#define DIFFUSE_LAMBERT             0
#define DIFFUSE_BURLEY              1

// Specular BRDF
// Normal distribution functions
#define SPECULAR_D_GGX              0

// Visibility functions
#define SPECULAR_V_SMITH_GGX        0
#define SPECULAR_V_SMITH_GGX_FAST   1
#define SPECULAR_V_GGX_ANISOTROPIC  2
#define SPECULAR_V_KELEMEN          3
#define SPECULAR_V_NEUBELT          4

// Fresnel functions
#define SPECULAR_F_SCHLICK          0

// Configuration
#define BRDF_DIFFUSE                DIFFUSE_LAMBERT
#define BRDF_SPECULAR_D             SPECULAR_D_GGX
#define BRDF_SPECULAR_V             SPECULAR_V_SMITH_GGX
#define BRDF_SPECULAR_F             SPECULAR_F_SCHLICK

//------------------------------------------------------------------------------
// Specular BRDF implementations
//------------------------------------------------------------------------------
float d_ggx(float roughness, float NoH, const vec3 h)
{
    // Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"

    // In mediump, there are two problems computing 1.0 - NoH^2
    // 1) 1.0 - NoH^2 suffers floating point cancellation when NoH^2 is close to 1 (highlights)
    // 2) NoH doesn't have enough precision around 1.0
    // Both problem can be fixed by computing 1-NoH^2 in highp and providing NoH in highp as well

    // However, we can do better using Lagrange's identity:
    //      ||a x b||^2 = ||a||^2 ||b||^2 - (a . b)^2
    // since N and H are unit vectors: ||N x H||^2 = 1.0 - NoH^2
    // This computes 1.0 - NoH^2 directly (which is close to zero in the highlights and has
    // enough precision).
    // Overall this yields better performance, keeping all computations in mediump
    float one_minus_noh_squared = 1.0 - NoH * NoH;
    float a = NoH * roughness;
    float k = roughness / (one_minus_noh_squared + a * a);
    float d = k * k * (1.0 / PI);
    return saturate_mediump(d);
}

float v_smith_ggx_correlated(float roughness, float NoV, float NoL)
{
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    float a2 = roughness * roughness;
    // TODO: lambda_v can be pre-computed for all the lights, it should be moved out of this function
    float lambda_v = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float lambda_l = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    float v = 0.5 / (lambda_v + lambda_l);
    // a2=0 => v = 1 / 4*NoL*NoV   => min=1/4, max=+inf
    // a2=1 => v = 1 / 2*(NoL+NoV) => min=1/4, max=+inf
    // clamp to the maximum value representable in mediump
    return saturate_mediump(v);
}

float v_smith_ggx_correlated_fast(float roughness, float NoV, float NoL)
{
    // Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
    float v = 0.5 / mix(2.0 * NoL * NoV, NoL + NoV, roughness);
    return saturate_mediump(v);
}

vec3 f_schlick(const vec3 f0, float f90, float VoH)
{
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

float f_schlick(float f0, float f90, float VoH)
{
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

//------------------------------------------------------------------------------
// Specular BRDF dispatch
//------------------------------------------------------------------------------
float distribution(float roughness, float NoH, const vec3 h)
{
#if BRDF_SPECULAR_D == SPECULAR_D_GGX
    return d_ggx(roughness, NoH, h);
#endif
}

float visibility(float roughness, float NoV, float NoL)
{
#if BRDF_SPECULAR_V == SPECULAR_V_SMITH_GGX
    return v_smith_ggx_correlated(roughness, NoV, NoL);
#elif BRDF_SPECULAR_V == SPECULAR_V_SMITH_GGX_FAST
    return v_smith_ggx_correlated_fast(roughness, NoV, NoL);
#endif
}

vec3 fresnel(const vec3 f0, float LoH)
{
#if BRDF_SPECULAR_F == SPECULAR_F_SCHLICK
    float f90 = saturate(dot(f0, vec3(50.0 * 0.33)));
    return f_schlick(f0, f90, LoH);
#endif
}

//------------------------------------------------------------------------------
// Diffuse BRDF implementations
//------------------------------------------------------------------------------
float fd_lambert()
{
    return 1.0 / PI;
}

float fd_burley(float roughness, float NoV, float NoL, float LoH)
{
    // Burley 2012, "Physically-Based Shading at Disney"
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float light_scatter = f_schlick(1.0, f90, NoL);
    float view_scatter  = f_schlick(1.0, f90, NoV);
    return light_scatter * view_scatter * (1.0 / PI);
}

//------------------------------------------------------------------------------
// Diffuse BRDF dispatch
//------------------------------------------------------------------------------
float diffuse(float roughness, float NoV, float NoL, float LoH)
{
#if BRDF_DIFFUSE == DIFFUSE_LAMBERT
    return fd_lambert();
#elif BRDF_DIFFUSE == DIFFUSE_BURLEY
    return fd_burley(roughness, NoV, NoL, LoH);
#endif
}

#endif
