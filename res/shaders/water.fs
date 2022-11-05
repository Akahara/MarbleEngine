#version 330 core


out vec4 color;

in vec2 o_uv;
in vec2 o_texCoordsWater;

in vec4 o_clipspace;


uniform sampler2D u_RefractionTexture;
uniform sampler2D u_ReflectionTexture;
uniform sampler2D u_dudvMap;

const float waveStrength = 0.02;

void main() 
{
    vec2 screenspace = ((o_clipspace.xy/o_clipspace.w)+1) * 0.5;

    vec2 distortion = texture(u_dudvMap, vec2(o_uv.x, o_uv.y)/2.0 + 0.5).rg * 2.f - 1.f * waveStrength;

    vec2 refract = vec2(screenspace.x,  screenspace.y);
    vec2 reflect = vec2(screenspace.x,  1-screenspace.y);

    //refract += distortion;
    //reflect += distortion;

    vec4 texReflec = texture(u_ReflectionTexture, reflect);

    vec4 texRefrac = texture(u_RefractionTexture, refract);
    color = mix(texRefrac, texReflec, 0.5);
    //color =  texture(u_dudvMap, distortion);
    //color = vec4(1,0,0,1);
}