#version 330 core

layout(location = 0) out vec4 color;

in vec2 o_uv;
in vec3 o_pos;
in vec3 o_normal


//uniform sampler2D u_texture;
uniform sampler2D u_depth;


void main()
{

    color.rgb = texture(u_depth, o_uv).rgb;
    color.a = 1;
    //color.r += 0.2;

}