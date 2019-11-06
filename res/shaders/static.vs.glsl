#version 330
in vec3 position;
in vec3 normal;
in vec2 uv0;

out vec3 color;
out vec2 uv;

uniform mat4 mvp;

void main()
{
    color = position;
    uv = uv0;
    gl_Position = mvp * vec4(position, 1.0);
}
