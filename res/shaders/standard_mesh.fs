#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in float o_texId;
in vec3 o_color;

uniform vec3 u_SunPos;
uniform float u_Strenght;
uniform int u_RenderChunks;

uniform sampler2D u_Textures2D[8];
int u_TexID;
//uniform int u_TexID;

vec3 sun_dir = normalize(u_SunPos);


void main()
{
       
    if (u_RenderChunks==0) {
        int index = int(o_texId);
        //index = 1;
        color = texture(u_Textures2D[index], o_uv); // TODO fix the texture sampling (use u_Textures2D)
        color.rgb += vec3(0.09,0.09,0.09) * 0.2 + vec3(.2f, .2f, 0.f) * dot(normalize(o_normal), normalize(u_SunPos)) * u_Strenght;
    }
    else {
        color = vec4(o_color.r, o_color.g, o_color.b, 1.f);
         color.rgb += vec3(0.09,0.09,0.09) * 0.2 + vec3(.2f, .2f, 0.f) * dot(normalize(o_normal), normalize(u_SunPos)) * u_Strenght;   
    }
}