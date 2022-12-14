#version 330 core

layout(location = 0) out vec4 color;

in vec2 o_TexCoord;
in float o_TexIndex;

uniform sampler2D u_Textures[12];

uniform float u_brightness = 1;

void main()
{
	int index = int(o_TexIndex);
	color = texture(u_Textures[index], o_TexCoord);
	color.a *= u_brightness * 0.25f;
};