#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;


vec3 sun_dir = normalize(vec3(45,10,3));
uniform float u_Strenght;

uniform sampler2D u_Texture2D;

void main()
{
    color = texture(u_Texture2D, o_uv);
    color.rgb -= dot(o_normal, sun_dir) * u_Strenght;

}
