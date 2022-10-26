#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in float i_texId;
layout(location = 4) in vec3 i_color;

out vec2 o_uv;
out vec3 o_normal;
out vec3 o_pos;
flat out float o_texId;
out vec3 o_color;

uniform mat4 u_VP;
uniform mat4 u_M;

void main()
{
  o_texId = i_texId;
  o_color = i_color;
  o_pos = (u_M * vec4(i_position,1)).xyz;
  o_uv = i_uv;
  o_normal = i_normal;
  gl_Position = u_VP * u_M * vec4(i_position, +1.0);
}
