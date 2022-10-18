#version 330 core

uniform mat4 u_VP;
uniform vec3 u_from, u_to;

void main()
{
  vec3 position = gl_VertexID == 0 ? u_from : u_to;
  gl_Position = u_VP * vec4(position, +1.0);
}
