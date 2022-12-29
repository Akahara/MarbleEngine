#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform int       u_RenderChunks = 0;
uniform vec2      u_grassSteepness = vec2(0.015f);
uniform sampler2D u_Textures2D[8];

vec4 computeBaseColor(vec3 normal) {
    // regular texture sample
    if (u_RenderChunks==0) {
        vec4 rockSample = texture(u_Textures2D[0], o_uv);
        vec4 grassSample = texture(u_Textures2D[1], o_uv);
        return mix(rockSample, grassSample, smoothstep(u_grassSteepness.x, u_grassSteepness.y, normal.y));
    } else {
        return vec4(o_color, 1.f);
    }
}