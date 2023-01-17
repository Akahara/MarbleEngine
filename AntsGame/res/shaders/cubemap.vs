#version 330 core

layout(location = 0) in vec3 i_position;

out vec3 v_texCoords;

uniform mat4 u_VP;
uniform vec3 u_displacement;

void main()
{
  v_texCoords = i_position;
  gl_Position = u_VP * vec4(i_position + u_displacement, +1.0);
  gl_Position.z = gl_Position.w; // force the cubemap to be rendered behind every other drawn element, see the cubemap renderer
  gl_ClipDistance[0] = +1;
}