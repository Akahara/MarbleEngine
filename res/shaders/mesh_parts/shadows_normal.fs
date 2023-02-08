#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

// This tells what texture slot is the normal texture of the index-Texture
uniform int u_NormalsTextureSlot[8];
uniform vec3 u_SunPos;

float computeSunlight(vec3 normal) {
    return max(0, dot(normalize(normal), normalize(u_SunPos)));
}