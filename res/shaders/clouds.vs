#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;

out vec2 o_uv;

uniform mat4 u_VP;
uniform mat4 u_M;

void main()
{
  o_uv = i_uv;
  gl_Position = u_VP * u_M * vec4(i_position, +1.0);
  gl_Position.z = gl_Position.w; // draw behind everything else
}