#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform sampler2D u_Textures2D[8];

uniform float u_Strength = 1.25f;
uniform vec3 u_SunPos = vec3(1000.f);
uniform int  u_RenderChunks = 0;
uniform vec2 u_grassSteepness = vec2(0.015f);
uniform vec3 u_fogColor = vec3(.71, .86, 1.);
uniform vec3 u_fogDamping = vec3(.001, .001, .001);

uniform vec3 u_cameraPos;

void main()
{
    if (u_RenderChunks==0) {
        vec4 rockSample = texture(u_Textures2D[0], o_uv);
        vec4 grassSample = texture(u_Textures2D[1], o_uv);
        color = mix(rockSample, grassSample, smoothstep(u_grassSteepness.x, u_grassSteepness.y, o_normal.y));
    } else {
        color = vec4(o_color.r, o_color.g, o_color.b, 1.f);
    }
    float sunStrength = dot(normalize(o_normal), normalize(u_SunPos))*u_Strength * .5 + .5;
    color.rgb *= mix(vec3(0.09,0.09,0.09), vec3(1., 1., .8), sunStrength);
    color.rgb = mix(u_fogColor, color.rgb, exp(-length(o_pos - u_cameraPos)*u_fogDamping));

    // normal only (works well with eroded terrain)
    //color.rgb = mix(vec3(0.282, 0.294, 0.294), vec3(0.894, 0.824, 0.667), dot(normalize(o_normal), normalize(u_SunPos)));
}