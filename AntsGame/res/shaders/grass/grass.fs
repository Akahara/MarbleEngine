#version 330 core

out vec4 color;

in float o_fragmentHeight;
in float o_colorPalette;

const vec3 bladeColor1 = vec3(0, .14, 0);
const vec3 bladeColor2 = vec3(.05, .8, .41);
const vec3 accent = vec3(0.906, 0.791, 0.340);

void main()
{
  color.rgb = mix(bladeColor1, bladeColor2, o_fragmentHeight);
  color.rgb = mix(color.rgb, accent, .1*sin(o_fragmentHeight));
  color.a = 1;
}
