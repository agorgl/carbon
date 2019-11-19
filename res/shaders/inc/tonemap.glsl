//
// tonemap.glsl
//

#include <common>
#include <math>

//------------------------------------------------------------------------------
// Tone-mapping configuration
//------------------------------------------------------------------------------
// Operators for LDR output
#define TONE_MAPPING_UNREAL           0
#define TONE_MAPPING_FILMIC_ALU       1
#define TONE_MAPPING_LINEAR           2 // Operators with built-in sRGB go above
#define TONE_MAPPING_REINHARD         3
#define TONE_MAPPING_ACES             4

// Operators for HDR output
#define TONE_MAPPING_ACES_REC2020_1K  5

// Debug operators
#define TONE_MAPPING_DISPLAY_RANGE    9

// Configuration
#define TONE_MAPPING_OPERATOR         TONE_MAPPING_ACES

//------------------------------------------------------------------------------
// Tone-mapping operators for LDR output
//------------------------------------------------------------------------------
vec3 tonemap_linear(const vec3 x)
{
    return x;
}

vec3 tonemap_reinhard(const vec3 x)
{
    // Reinhard et al. 2002, "Photographic Tone Reproduction for Digital Images", Eq. 3
    return x / (1.0 + luminance(x));
}

vec3 tonemap_unreal(const vec3 x)
{
    // Unreal, Documentation: "Color Grading"
    // Adapted to be close to Tonemap_ACES, with similar range
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    return x / (x + 0.155) * 1.019;
}

vec3 tonemap_filmic_alu(const vec3 x)
{
    // Hable 2010, "Filmic Tonemapping Operators"
    // Based on Duiker's curve, optimized by Hejl and Burgess-Dawson
    // Gamma 2.2 correction is baked in, don't use with sRGB conversion!
    vec3 c = max(vec3(0.0), x - 0.004);
    return (c * (c * 6.2 + 0.5)) / (c * (c * 6.2 + 1.7) + 0.06);
}

vec3 tonemap_aces(const vec3 x)
{
    // Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

//------------------------------------------------------------------------------
// Tone-mapping operators for HDR output
//------------------------------------------------------------------------------
#if TONE_MAPPING_OPERATOR == TONE_MAPPING_ACES_REC2020_1K
vec3 tonemap_aces_rec2020_1k(const vec3 x)
{
    // Narkowicz 2016, "HDR Display – First Steps"
    const float a = 15.8;
    const float b = 2.12;
    const float c = 1.2;
    const float d = 5.92;
    const float e = 1.9;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}
#endif

//------------------------------------------------------------------------------
// Debug tone-mapping operators, for LDR output
//------------------------------------------------------------------------------
/**
 * Converts the input HDR RGB color into one of 16 debug colors that represent
 * the pixel's exposure. When the output is cyan, the input color represents
 * middle gray (18% exposure). Every exposure stop above or below middle gray
 * causes a color shift.
 *
 * The relationship between exposures and colors is:
 *
 * -5EV  - black
 * -4EV  - darkest blue
 * -3EV  - darker blue
 * -2EV  - dark blue
 * -1EV  - blue
 *  OEV  - cyan
 * +1EV  - dark green
 * +2EV  - green
 * +3EV  - yellow
 * +4EV  - yellow-orange
 * +5EV  - orange
 * +6EV  - bright red
 * +7EV  - red
 * +8EV  - magenta
 * +9EV  - purple
 * +10EV - white
 */
#if TONE_MAPPING_OPERATOR == TONE_MAPPING_DISPLAY_RANGE
vec3 tonemap_display_range(const vec3 x)
{
    // 16 debug colors + 1 duplicated at the end for easy indexing
    const vec3 debug_colors[17] = vec3[](
         vec3(0.0, 0.0, 0.0),         // black
         vec3(0.0, 0.0, 0.1647),      // darkest blue
         vec3(0.0, 0.0, 0.3647),      // darker blue
         vec3(0.0, 0.0, 0.6647),      // dark blue
         vec3(0.0, 0.0, 0.9647),      // blue
         vec3(0.0, 0.9255, 0.9255),   // cyan
         vec3(0.0, 0.5647, 0.0),      // dark green
         vec3(0.0, 0.7843, 0.0),      // green
         vec3(1.0, 1.0, 0.0),         // yellow
         vec3(0.90588, 0.75294, 0.0), // yellow-orange
         vec3(1.0, 0.5647, 0.0),      // orange
         vec3(1.0, 0.0, 0.0),         // bright red
         vec3(0.8392, 0.0, 0.0),      // red
         vec3(1.0, 0.0, 1.0),         // magenta
         vec3(0.6, 0.3333, 0.7882),   // purple
         vec3(1.0, 1.0, 1.0),         // white
         vec3(1.0, 1.0, 1.0)          // white
    );

    // The 5th color in the array (cyan) represents middle gray (18%)
    // Every stop above or below middle gray causes a color shift
    float v = log2(luminance(x) / 0.18);
    v = clamp(v + 5.0, 0.0, 15.0);
    int index = int(v);
    return mix(debug_colors[index], debug_colors[index + 1], v - float(index));
}
#endif

//------------------------------------------------------------------------------
// Tone-mapping dispatch
//------------------------------------------------------------------------------
/**
 * Tone-maps the specified RGB color. The input color must be in linear HDR and
 * pre-exposed. Our HDR to LDR tone mapping operators are designed to tone-map
 * the range [0..~8] to [0..1].
 */
vec3 tonemap(const vec3 x)
{
#if TONE_MAPPING_OPERATOR == TONE_MAPPING_UNREAL
    return tonemap_unreal(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_FILMIC_ALU
    return tonemap_filmic_alu(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_LINEAR
    return tonemap_linear(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_REINHARD
    return tonemap_reinhard(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_ACES
    return tonemap_aces(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_ACES_REC2020_1K
    return tonemap_aces_rec2020_1k(x);
#elif TONE_MAPPING_OPERATOR == TONE_MAPPING_DISPLAY_RANGE
    return tonemap_display_range(x);
#endif
}

//------------------------------------------------------------------------------
// Inverse tone-mapping operations
//------------------------------------------------------------------------------
/*
 * The input must be in the [0, 1] range.
 */
vec3 inverse_tonemap_unreal(const vec3 x)
{
    return (x * -0.155) / (x - 1.019);
}

/**
 * Applies the inverse of the tone mapping operator to the specified HDR or LDR
 * sRGB (non-linear) color and returns a linear sRGB color. The inverse tone mapping
 * operator may be an approximation of the real inverse operation.
 */
vec3 inverse_tonemap_srgb(vec3 color)
{
    // sRGB input
    color = clamp(color, 0.0, 1.0);
    return inverse_tonemap_unreal(color);
}

/**
 * Applies the inverse of the tone mapping operator to the specified HDR or LDR
 * linear RGB color and returns a linear RGB color. The inverse tone mapping operator
 * may be an approximation of the real inverse operation.
 */
vec3 inverse_tonemap(vec3 linear)
{
    // Linear input
    linear = clamp(linear, 0.0, 1.0);
    return inverse_tonemap_unreal(pow(linear, vec3(1.0 / 2.2)));
}

//------------------------------------------------------------------------------
// Processing tone-mappers
//------------------------------------------------------------------------------
vec3 tonemap_reinhard_weighted(const vec3 x, float weight)
{
    // Weighted Reinhard tone-mapping operator designed for post-processing
    // This tone-mapping operator is invertible
    return x * (weight / (max3(x) + 1.0));
}

vec3 tonemap_reinhard_weighted_invert(const vec3 x)
{
    // Inverse Reinhard tone-mapping operator, designed to be used in conjunction
    // with the weighted Reinhard tone-mapping operator
    return x / (1.0 - max3(x));
}
