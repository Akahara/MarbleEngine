#version 330 core

out vec4 color;

in float o_fragmentHeight;

const vec3 bladeColor1 = vec3(0, .14, 0);
const vec3 bladeColor2 = vec3(.05, .8, .41);

void main()
{
  // TODO color the grass
  color = vec4(mix(bladeColor1, bladeColor2, o_fragmentHeight), 1);
}
