#version 330 core

// Vec4 to render
layout (location=0) out vec4 gAlbedo;
layout (location=1) out vec4 gNormal;
layout (location=2) out vec4 gPosition;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;
flat in int o_texId;

// Make a factory out of this for insane over-engineering

void main() 
{

    gPosition = vec4(o_pos,1);
    gNormal = vec4(o_normal,1);
    gAlbedo = vec4((o_color)*3.F,1);

}