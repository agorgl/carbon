//
// octahedral.glsl
//
#ifndef _OCTAHEDRAL_GLSL_
#define _OCTAHEDRAL_GLSL_

/**
 * Efficient GPU implementation of the octahedral unit vector encoding from
 *
 * Cigolle, Donow, Evangelakos, Mara, McGuire, Meyer,
 * A Survey of Efficient Representations for Independent Unit Vectors, Journal of Computer Graphics Techniques (JCGT), vol. 3, no. 2, 1-30, 2014
 * Available online http://jcgt.org/published/0003/02/01/
 */

// Returns 1.0 if the given value is positive or zero, and -1.0 if it is negative.
// This is similar to the GLSL built-in function sign,
// except that returns 1.0 instead of 0.0 when the input value is 0.0.
float sign_not_zero(in float k)
{
    return (k >= 0.0) ? 1.0 : -1.0;
}

// Similar to the above, augmented to the vec2 datatype
vec2 sign_not_zero(in vec2 v)
{
    return vec2(sign_not_zero(v.x), sign_not_zero(v.y));
}

// Assumes that v is a unit vector.
// The result is an octahedral vector on the [-1, +1] square.
vec2 oct_encode(in vec3 v)
{
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    vec2 result = v.xy * (1.0 / l1norm);
    if (v.z < 0.0) {
        result = (1.0 - abs(result.yx)) * sign_not_zero(result.xy);
    }
    return result;
}

// Returns a unit vector.
// Argument o is an octahedral vector packed via oct_encode, on the [-1, +1] square.
vec3 oct_decode(vec2 o)
{
    vec3 v = vec3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
    if (v.z < 0.0) {
        v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
    }
    return normalize(v);
}

#endif
