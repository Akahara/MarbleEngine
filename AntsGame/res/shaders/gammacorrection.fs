#version 330 core

out vec4 color;

in vec2 o_uv;


uniform sampler2D u_texture;

uniform float u_gamma = 2.2f;


void main()
{
   
    vec2 uv = o_uv;
    color = texture(u_texture, uv);
    color.xyz = pow(color.xyz, vec3(1.0/u_gamma));
    color.a = 1;
    
}
