#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec3 i_normal;
layout(location = 3) in vec3 i_color;
layout(location = 4) in float i_texId;
// instance members
layout(location = 5) in vec3 i_iposition;
layout(location = 6) in vec3 i_iscale;
layout(location = 7) in float i_irotation;

out vec2 o_uv;
out vec3 o_normal;
out vec3 o_pos;
out vec3 o_color;
flat out int o_texId;

// water.fs only output variables (TODO use water.vs instead of this shader to avoid bloating this shader)
out vec2 o_texCoordsWater;
out vec3 o_SunPos;
out vec4 o_clipspace;
out vec3 o_toCameraVector;
out vec3 o_fromLightVector;

uniform mat4 u_VP;
uniform vec3 u_SunPos = vec3(1000.f);

uniform vec3 u_camPos = vec3(0.f,0.f,0.f);
uniform vec4 u_plane = vec4(0, -1, 0, 10000);

void main()
{
  vec4 worldPos = vec4(i_position, 0);
  //worldPos.xz += i_irotation * (-worldPos.xz -worldPos.zw +worldPos.wx);
  worldPos.xyz *= i_iscale;
  worldPos.xyz += i_iposition;
  worldPos.w = 1;
  gl_ClipDistance[0] = dot(worldPos, u_plane);
  gl_ClipDistance[0] = 100000;
  vec4 screenSpacePos = u_VP * worldPos;

  o_pos = worldPos.xyz;
  o_uv = i_uv;
  o_normal = i_normal;
  o_color = i_color;
  o_texId = int(i_texId);

  o_toCameraVector = u_camPos - o_pos;
  o_SunPos = u_SunPos;
  o_fromLightVector = o_pos - u_SunPos;
  o_clipspace = screenSpacePos;

  gl_Position = screenSpacePos;
  
}