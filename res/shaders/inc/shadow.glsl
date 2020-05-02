//
// shadow.glsl
//
#ifndef _SHADOW_GLSL_
#define _SHADOW_GLSL_

const float vsm_bias = 0.001;
const float light_bleeding_reduction = 0.1;
const float positive_exponent = 40.0;
const float negative_exponent = 8.0;

vec2 evsm_exponents(float positive_exponent, float negative_exponent, vec3 cascade_scale)
{
    const float max_exponent = 42.0; // 5.54 for 16bit
    vec2 light_space_exponents = vec2(positive_exponent, negative_exponent);

    // Make sure exponents say consistent in light space regardless of partition
    // scaling. This prevents the exponentials from ever getting too rediculous
    // and maintains consistency across partitions.
    // Clamp to maximum range of fp32/fp16 to prevent overflow/underflow
    return min(light_space_exponents / cascade_scale.z, max_exponent);
}

// Applies exponential warp to shadow map depth, input depth should be in [0, 1]
vec2 warp_depth(float depth, vec2 exponents)
{
    // Rescale depth into [-1, 1]
    depth = 2.0 * depth - 1.0;
    float pos =  exp( exponents.x * depth);
    float neg = -exp(-exponents.y * depth);
    return vec2(pos, neg);
}

float linstep(float a, float b, float v)
{
    return clamp((v - a) / (b - a), 0.0, 1.0);
}

// Reduces VSM light bleedning
float reduce_light_bleeding(float p_max, float amount)
{
    // Remove the [0, amount] tail and linearly rescale (amount, 1].
    return linstep(amount, 1.0, p_max);
}

float chebyshev_upper_bound(vec2 moments, float mean, float min_variance, float light_bleeding_reduction)
{
    // Compute variance
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, min_variance);

    // Compute probabilistic upper bound
    float d = mean - moments.x;
    float p_max = variance / (variance + (d * d));

    p_max = reduce_light_bleeding(p_max, light_bleeding_reduction);

    // One-tailed Chebyshev
    return (mean <= moments.x ? 1.0 : p_max);
}

float sample_shadow_map_evsm(sampler2D evsm, vec3 shadow_pos, vec3 shadow_pos_dx, vec3 shadow_pos_dy, uint cascade_idx)
{
    //vec3 cascade_scales = cascade_scales[cascade_idx].xyz;
    //vec3 sampleco = vec3(shadow_pos.xy, cascade_idx);
    vec3 cascade_scales = vec3(1.0);
    vec2 sampleco = shadow_pos.xy;

    vec2 exponents = evsm_exponents(positive_exponent, negative_exponent, cascade_scales);
    vec2 warped_depth = warp_depth(shadow_pos.z, exponents);
    vec4 occluder = textureGrad(evsm, sampleco, shadow_pos_dx.xy, shadow_pos_dy.xy);

    // Derivative of warping at depth
    vec2 depth_scale = vsm_bias * 0.01 * exponents * warped_depth;
    vec2 min_variance = depth_scale * depth_scale;

    float pos_contrib = chebyshev_upper_bound(occluder.xz, warped_depth.x, min_variance.x, light_bleeding_reduction);
    float neg_contrib = chebyshev_upper_bound(occluder.yw, warped_depth.y, min_variance.y, light_bleeding_reduction);

    float shadow_contrib = pos_contrib;
    shadow_contrib = min(shadow_contrib, neg_contrib);
    return shadow_contrib;
}

float sample_shadow_map(sampler2D sm, vec3 shadow_pos, vec3 shadow_pos_dx, vec3 shadow_pos_dy, uint cascade_idx)
{
    return sample_shadow_map_evsm(sm, shadow_pos, shadow_pos_dx, shadow_pos_dy, cascade_idx);
}

float shadow_visibility(sampler2D sm, vec3 shadow_pos)
{
    vec3 shadow_pos_dx = dFdx(shadow_pos);
    vec3 shadow_pos_dy = dFdy(shadow_pos);
    float shadow_visibility = sample_shadow_map(sm, shadow_pos, shadow_pos_dx, shadow_pos_dy, 0u);
    return shadow_visibility;
}

#endif
