#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;




uniform vec3 u_SunPos;
uniform float u_Strenght;

uniform sampler2D u_Texture2D;

vec3 sun_dir = normalize(u_SunPos);


void main()
{
    color = texture(u_Texture2D, o_uv);
    color.rgb = color.rgb / 2;
    color.rgb -= dot(o_normal, -sun_dir) * u_Strenght;

}
