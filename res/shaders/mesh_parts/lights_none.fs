#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

vec4 computeExtraLights(vec3 normal) {
    return vec4(0);
}