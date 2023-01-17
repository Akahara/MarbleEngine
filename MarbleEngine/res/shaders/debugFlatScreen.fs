#version 330 core

layout(location = 0) out vec4 color;

in vec2 o_uv;
in vec3 o_pos;

uniform sampler2D u_texture;

void main()
{

    color = texture(u_texture, vec2(1-o_uv.x, 1-o_uv.y));
    color.a = 1;
    //color.r += 0.2;

}