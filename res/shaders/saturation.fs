#version 330 core

out vec4 color;

in vec2 o_uv;


uniform sampler2D u_texture;
uniform vec2 u_screenSize;

void main()
{
   
    
    vec2 uv = o_uv;
    color = 1 - texture(u_texture, uv);
    color = vec4(1.0f, 0.0f, 0.0f, 1.f);
    
}
