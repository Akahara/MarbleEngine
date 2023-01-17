#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform vec3 u_fogColor = vec3(.71, .86, 1.);
uniform vec3 u_fogDamping = vec3(.001, .001, .001);
uniform vec3 u_cameraPos;

void applyFinalPass(vec3 normal, inout vec4 color) {
    color.rgb = mix(u_fogColor, color.rgb, exp(-length(o_pos - u_cameraPos)*u_fogDamping));
}