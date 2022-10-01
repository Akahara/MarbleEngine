#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;

uniform sampler2D u_Texture2D;

void main()
{
    color = texture(u_Texture2D, o_uv);
}
