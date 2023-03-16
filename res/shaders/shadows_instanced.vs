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

out vec2 o_uv;
out vec3 o_color;
out vec3 o_normal; // FIX normalize standard VS and standard instanced VS outputs (o_xx vs v_xx)
                   // and also rename this file to standard_instanced.vs

uniform mat4 u_VP;

void main()
{
  vec4 position = vec4(m_position * i_scale + i_position, 1);
  v_vertex = m_position;
  v_normal = m_normal;
  v_position = position.xyz;
  o_uv = m_uv;
  o_color = vec3(1);
  o_normal = m_normal;
  gl_Position = u_VP * position;
}
