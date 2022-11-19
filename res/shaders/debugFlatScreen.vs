#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in float i_texId;
layout(location = 4) in vec3 i_color;


out vec2 o_uv;
out vec3 o_pos;

void main()
{
  o_pos = i_position;
  o_uv = i_uv;

  gl_Position = vec4(i_position,1);

}