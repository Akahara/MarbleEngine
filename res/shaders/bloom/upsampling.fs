#version 330 core

uniform sampler2D u_srcTexture;
uniform float u_filterRadius;

in vec2 texCoord;
layout (location = 0) out vec3 upsample;

void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = u_filterRadius;
    float y = u_filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(u_srcTexture, vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 b = texture(u_srcTexture, vec2(texCoord.x,     texCoord.y + y)).rgb;
    vec3 c = texture(u_srcTexture, vec2(texCoord.x + x, texCoord.y + y)).rgb;

    vec3 d = texture(u_srcTexture, vec2(texCoord.x - x, texCoord.y)).rgb;
    vec3 e = texture(u_srcTexture, vec2(texCoord.x,     texCoord.y)).rgb;
    vec3 f = texture(u_srcTexture, vec2(texCoord.x + x, texCoord.y)).rgb;

    vec3 g = texture(u_srcTexture, vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 h = texture(u_srcTexture, vec2(texCoord.x,     texCoord.y - y)).rgb;
    vec3 i = texture(u_srcTexture, vec2(texCoord.x + x, texCoord.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;
}