//
// common.glsl
//
#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

//------------------------------------------------------------------------------
// Common computation operations
//------------------------------------------------------------------------------
/**
 * Computes the pre-exposed intensity using the specified intensity and exposure.
 * This function exists to force highp precision on the two parameters
 */
float compute_preexposed_intensity(const highp float intensity, const highp float exposure)
{
    return intensity * exposure;
}

//------------------------------------------------------------------------------
// Common texture operations
//------------------------------------------------------------------------------
/**
 * Decodes the specified RGBM value to linear HDR RGB.
 */
vec3 decode_rgbm(vec4 c)
{
    c.rgb *= (c.a * 16.0);
    return c.rgb * c.rgb;
}

//------------------------------------------------------------------------------
// Common debug
//------------------------------------------------------------------------------
vec3 heatmap(float v)
{
    vec3 r = v * 2.1 - vec3(1.8, 1.14, 0.3);
    return 1.0 - r * r;
}

#endif
