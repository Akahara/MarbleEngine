#version 330 core

out vec4 color;

in vec2 o_uv;


uniform sampler2D u_texture;

uniform float u_contrastShift = 0.5f;

void main()
{
   
    vec2 uv = o_uv;
    color = texture(u_texture, uv);
    color.a = 1;

    //----------------------//

    color = 0.5 + (1.0 + u_contrastShift) * (color - 0.5);


    
}
