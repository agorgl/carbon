#version 330
in vec3 color;
in vec2 uv;

out vec4 fcolor;

uniform float textured;
uniform sampler2D tex;

void main()
{
    vec3 c = mix(color, texture(tex, uv).rgb, textured);
    fcolor = vec4(c, 1.0);
}
