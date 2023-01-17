#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform sampler2D u_Textures2D[8];

const vec3 mesa_colors[5] = vec3[5](
    vec3(0.725, 0.455, 0.102),
    vec3(0.949, 0.639, 0.369),
    vec3(0.941, 0.482, 0.412),
    vec3(0.867, 0.720, 0.698),
    vec3(0.867, 0.630, 0.325)
);

vec4 computeBaseColor(vec3 normal) {
    // for the mesa scene, a simple texture sample does not sufice
    return mix(texture(u_Textures2D[0], o_uv), vec4(mesa_colors[int(floor(o_pos.y*.5))%5], 1), .7);
}