#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in float o_texId;



uniform vec3 u_SunPos;
uniform float u_Strenght;

uniform sampler2D u_Textures2D[8];
uniform float delta;

vec3 sun_dir = normalize(u_SunPos);


void main()
{
    int index = int(o_texId);
    if (index != 0) {
        color = texture(u_Textures2D[index], o_uv);
    } else {
        color = texture(u_Textures2D[index], o_uv + vec2(delta,0));
        color.a = color.r;
        color.rgb = vec3(0.9);

    }
    // .2 + dot(normal, sun) * .8
    float ambiantLight = 0.2 + dot(o_normal, normalize(u_SunPos)) * 0.8;
    // ambiantColor + lerp(0, sun, dot(sunDir, normal))
    color.rgb += vec3(0.09,0.09,0.09) * 0.2 + vec3(.2f, .2f, 0.f) * dot(normalize(o_normal), normalize(u_SunPos)) * u_Strenght;} // (distance(u_SunPos, o_pos) * distance(u_SunPos, o_pos));}
    