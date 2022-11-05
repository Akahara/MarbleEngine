#version 430 core

layout(std430, binding = 1) buffer o_culled {
  bool culled_instances[];
};

layout(location = 0) in vec3 im_position;
layout(location = 1) in vec4 ii_position;
layout(location = 2) in float ii_colorPalette;

out float o_fragmentHeight;
out float o_colorPalette;

uniform mat4 u_VP;
uniform mat2 u_R;

void main()
{
  o_fragmentHeight = im_position.y;
  //o_colorPalette = ii_colorPalette;
  o_colorPalette = culled_instances[gl_InstanceID] ? 1 : 0;
  vec4 vertex = vec4(im_position, 1);
  vertex.xz = u_R * vertex.xz;
  vertex.y *= ii_position.w;
  vertex.xyz += ii_position.xyz;
  vertex = u_VP * vertex;
  gl_Position = vertex;
}
