//
// math.glsl
//

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
#define PI                  3.14159265359
#define HALF_PI             1.570796327
#define FLT_EPS             1e-5

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define saturate_mediump(x) x
#define saturate(x)         clamp(x, 0.0, 1.0)

//------------------------------------------------------------------------------
// Scalar operations
//------------------------------------------------------------------------------
// Computes x^5 using only multiply operations
float pow5(float x)
{
    float x2 = x * x;
    return x2 * x2 * x;
}

// Computes x^2 as a single multiplication
float sq(float x)
{
    return x * x;
}

// Returns the maximum component of the specified vector
float max3(const vec3 v)
{
    return max(v.x, max(v.y, v.z));
}
