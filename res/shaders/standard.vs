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

uniform vec3 u_camPos = vec3(0.f,0.f,0.f); // for water i guess
out vec3 o_toCameraVector;

const float tiling = 6.0f;

uniform vec4 u_plane = vec4(0, -1, 0, 10000);

void main()
{
  o_pos = (u_M * vec4(i_position,1)).xyz;
  o_clipspace = u_VP * u_M * vec4(i_position, +1.0);

  vec4 worldPos =  u_M * vec4(i_position, +1.0);

  gl_ClipDistance[0] = dot(worldPos, u_plane);

  o_color = i_color;
  o_uv = i_uv;
  o_normal = i_normal;

  o_toCameraVector = u_camPos - o_pos;
  //o_fromLightVector = o_pos - u_SunPos; // TODO cleanup shaders

  gl_Position =  o_clipspace;
}
