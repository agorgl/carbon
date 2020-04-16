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

/**
 * Encodes a floating point number in [0..1) range
 * into several channels of 8 bit/channel render texture.
 */
vec4 encode_depth(float v)
{
    vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
    enc  = fract(enc);
    enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
    return enc;
}

/**
 * Decodes an 8 bit/channel value
 * into a floating point number in [0..1) range.
 */
float decode_depth(vec4 rgba)
{
    return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
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
