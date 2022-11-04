#version 330 core

layout(location = 0) out vec4 color;

in vec2 o_uv;
in vec3 o_pos;

uniform sampler2D u_texture;

void main()
{

    color = texture(u_texture, o_uv);
    color.a = 1;
    color.r += 0.2;

}