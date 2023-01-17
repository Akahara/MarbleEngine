#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform float u_Strength = 1.25f;

void adjustNormal(inout vec3 normal);
vec4 computeBaseColor(vec3 normal);
float computeSunlight(vec3 normal);
vec4 computeExtraLights(vec3 normal);
void applyFinalPass(vec3 normal, inout vec4 color);

void main()
{
    vec3 normal = o_normal;
    adjustNormal(normal);
    color = computeBaseColor(normal);
    color.rgb *= mix(vec3(0.09,0.09,0.09), vec3(1., 1., .8), max(.1, computeSunlight(normal)*u_Strength));
    color += computeExtraLights(normal);
    applyFinalPass(normal, color);
}
