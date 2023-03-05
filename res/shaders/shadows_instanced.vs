#version 330 core

// model
layout(location=0) in vec3 m_position;
layout(location=1) in vec2 m_uv;
layout(location=2) in vec3 m_normal;
// instance
layout(location=5) in vec3 i_position;
layout(location=6) in vec3 i_scale;

out vec3 v_normal;
out vec3 v_vertex;

out vec3 v_position;

uniform mat4 u_VP;

void main()
{
  v_vertex = m_position;
  v_normal = m_normal;
  vec4 position = vec4(m_position * i_scale + i_position, 1);
  v_position = position.xyz;
  gl_Position = u_VP * position;
}
