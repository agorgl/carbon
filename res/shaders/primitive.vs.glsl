#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anrm;
layout (location = 2) in vec2 atco;
layout (location = 3) in vec4 atng;

out vec3 vpos;
out vec2 vtco;
out mat3 vtbn;

uniform mat4 modl;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    // Calculate vertex world position
    vec3 wpos = vec3(modl * vec4(apos, 1.0));

    // Calculate the normal and tangent
    vec3 n = normalize((modl * vec4(anrm, 0.0)).xyz);
    vec3 t = normalize((modl * vec4(atng.xyz, 0.0)).xyz);

    // Reconstruct the bitangent from the normal and tangent
    vec3 b = normalize((modl * vec4(cross(anrm, atng.xyz) * sign(atng.w), 0.0)).xyz);

    // Construct tbn matrix
    mat3 tbn = mat3(t, b, n);

    // Populate vertex outputs
    vpos = wpos;
    vtco = atco;
    vtbn = tbn;

    // Fill output position
    gl_Position = proj * view * modl * vec4(apos, 1.0);
}
