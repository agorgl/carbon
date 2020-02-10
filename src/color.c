#include <color.h>

typedef vec3 xyY;
typedef vec3 XYZ;

static XYZ xyY_to_XYZ(xyY v)
{
    return (XYZ){{v.x / v.y, v.z, (1.0f - v.x - v.y) / v.y}};
}

static vec3 XYZ_to_sRGB(XYZ v)
{
    // XYZ to linear sRGB
    const mat3 XYZ_sRGB = (mat3){{
        3.2404542f, -0.9692660f,  0.0556434f,
       -1.5371385f,  1.8760108f, -0.2040259f,
       -0.4985314f,  0.0415560f,  1.0572252f
    }};
    return mat3_mul_vec3(XYZ_sRGB, v);
}

#define vec3_max(v) (max(max(v.x, v.y), v.z))

vec3 cct(float K)
{
    // Temperature to CIE 1960
    float K2 = K * K;
    float u = (0.860117757f + 1.54118254e-4f * K + 1.28641212e-7f * K2)
            / (1.0f + 8.42420235e-4f * K + 7.08145163e-7f * K2);
    float v = (0.317398726f + 4.22806245e-5f * K + 4.20481691e-8f * K2)
            / (1.0f - 2.89741816e-5f * K + 1.61456053e-7f * K2);
    float d = 1.0f / (2.0f * u - 8.0f * v + 4.0f);
    vec3 linear = XYZ_to_sRGB(xyY_to_XYZ((xyY){{3.0f * u * d, 2.0f * v * d, 1.0f}}));

    // Normalize and saturate
    return vec3_saturate(vec3_div(linear, max(1e-5f, vec3_max(linear))));
}

vec3 illuminantD(float K)
{
    // Temperature to xyY
    const float iK = 1.0f / K;
    float iK2 = iK * iK;
    float x = K <= 7000.0f
        ? 0.244063f + 0.09911e3f * iK + 2.9678e6f * iK2 - 4.6070e9f * iK2 * iK
        : 0.237040f + 0.24748e3f * iK + 1.9018e6f * iK2 - 2.0064e9f * iK2 * iK;
    float y = -3.0f * x * x + 2.87f * x - 0.275f;
    vec3 linear = XYZ_to_sRGB(xyY_to_XYZ((xyY){{x, y, 1.0f}}));

    // Normalize and saturate
    return vec3_saturate(vec3_div(linear, max(1e-5f, vec3_max(linear))));
}
