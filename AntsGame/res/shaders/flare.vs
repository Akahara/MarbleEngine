#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout (location = 2) in float a_TexIndex;

out vec2 o_TexCoord;
out float o_TexIndex;

void main(){

	o_TexIndex = a_TexIndex;
	o_TexCoord = texCoord;


	gl_Position = vec4(position,1.0f);
};