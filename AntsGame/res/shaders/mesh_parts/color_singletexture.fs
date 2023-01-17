#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform sampler2D u_Textures2D[8];

vec4 computeBaseColor(vec3 normal) {
    return texture(u_Textures2D[0], o_uv);
}