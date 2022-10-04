#version 330 core

out vec4 color;

in vec2 o_uv;

uniform sampler2D u_texture;

void main()
{
    vec2 uv = o_uv;
    color = texture(u_texture, uv);
    color.a = 1;
}
