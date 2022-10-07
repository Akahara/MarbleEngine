#version 330 core

layout(location=0) in vec3 i_position;
layout(location=1) in vec2 i_uv;
layout(location=2) in vec3 i_normal;

out vec3 v_normal;
out vec3 v_vertex;

out vec3 v_position;

uniform mat4 u_M;
uniform mat4 u_VP;

void main()
{
  v_vertex = i_position;
  v_normal = i_normal;
  v_position = (u_M * vec4(i_position, 1)).xyz;
  gl_Position = u_VP * vec4(v_position, 1);
}
