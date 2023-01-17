#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;

uniform float t_test;
uniform vec2 t_foo;

void main()
{
    vec2 uv = o_uv;
    color = vec4(uv, 0, 1);
    color += .5 - t_test + t_foo.x;
}
