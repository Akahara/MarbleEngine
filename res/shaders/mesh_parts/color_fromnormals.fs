#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform vec3 u_SunPos = vec3(1000.f);

vec4 computeBaseColor(vec3 normal) {
    // normal only (works well with eroded terrain)
    return mix(vec4(0.282, 0.294, 0.294, 1), vec3(0.894, 0.824, 0.667, 1),
        dot(normalize(normal), normalize(u_SunPos)));
}