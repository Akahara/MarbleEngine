#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
flat in float o_texId;
in vec3 o_color;

uniform vec3 u_SunPos = vec3(100.f);
uniform float u_Strength = 1.25f;
uniform int u_RenderChunks = 0;
uniform vec3 u_cameraPos;
uniform vec2 u_grassSteepness = vec2(0.015f);

uniform sampler2D u_Textures2D[8];

uniform vec3 u_fogColor = vec3(.71, .86, 1.);
uniform vec3 u_fogDamping = vec3(.001, .001, .001);


void main()
{
    if (u_RenderChunks==0) {
        int index = int(o_texId);
        vec4 rockSample = texture(u_Textures2D[0], o_uv);
        vec4 grassSample = texture(u_Textures2D[1], o_uv);
        color = mix(rockSample, grassSample, smoothstep(u_grassSteepness.x, u_grassSteepness.y, o_normal.y));


    } else {
        color = vec4(o_color.r, o_color.g, o_color.b, 1.f);
    }
    color.rgb += vec3(0.09,0.09,0.09) * 0.2 + vec3(.2f, .2f, 0.f) * dot(normalize(o_normal), normalize(u_SunPos)) * u_Strength;
    color.rgb = mix(u_fogColor, color.rgb, exp(-length(o_pos - u_cameraPos)*u_fogDamping));
}