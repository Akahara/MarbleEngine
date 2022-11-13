#version 430 core

layout(location = 0) in vec3 im_position;
layout(location = 1) in vec4 ii_position;

out float o_fragmentHeight;
out float o_colorPalette;

uniform mat4 u_VP;
uniform mat2 u_R;
uniform float u_time;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
float noise(vec2 n) {
  const vec2 d = vec2(0.0, 1.0);
  vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
  return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

void main()
{
  o_fragmentHeight = im_position.y;
  vec4 vertex = vec4(im_position, 1);
  vertex.xz = u_R * vertex.xz;
  float h = ii_position.w;
  vertex.y *= h;
  vertex.xz += vertex.y * (cos(u_time + (ii_position.x + ii_position.z)*.2 + rand(ii_position.xz))*.1+.15);
  vertex.xyz += ii_position.xyz;
  vertex = u_VP * vertex;
  gl_Position = vertex;
}
