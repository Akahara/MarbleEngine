#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;

void main()
{
    vec2 uv = o_uv;
    color = vec4(uv, 0, 1);
    color += .5;
}
