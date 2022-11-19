#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in vec3 i_color;

out vec2 o_uv;
out vec3 o_normal;
out vec3 o_pos;
out vec3 o_color;

out vec4 o_clipspace;
out vec2 o_texCoordsWater;
out vec3 o_fromLightVector; // water normal lighting stuff

uniform mat4 u_VP;
uniform mat4 u_M;

uniform vec4 u_plane = vec4(0, -1, 0, 10000); // TODO cleanup shaders

void main()
{
  vec4 worldPos = u_M * vec4(i_position, +1.0);

  o_pos = worldPos.xyz;
  o_clipspace = u_VP * worldPos;
  o_color = i_color;
  o_uv = i_uv;
  o_normal = i_normal;

  gl_ClipDistance[0] = dot(worldPos, u_plane);

  gl_Position = o_clipspace;
}
