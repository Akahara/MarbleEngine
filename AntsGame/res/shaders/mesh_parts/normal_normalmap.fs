#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

// This tells what texture slot is the normal texture of the index-Texture
uniform int u_NormalsTextureSlot[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
uniform sampler2D u_Textures2D[8];

void adjustNormal(inout vec3 normal) {
    if (u_NormalsTextureSlot[0] != -1) {
        int indexNormalSlot = u_NormalsTextureSlot[0];
        vec4 sn = texture(u_Textures2D[indexNormalSlot],o_uv);
        normal = normal * sn.rgb;
    }
}