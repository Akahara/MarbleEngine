#version 330 core

out vec4 color;

in vec2 o_uv;


uniform sampler2D u_texture;
uniform float u_saturation = 1.2f;

float computeLuminance(vec3 color) {

    float r = color.x * 0.2125;
    float g = color.y * 0.7153;
    float b = color.z * 0.07121;

    return r + g + b;

}

float fastLuminance(vec3 color) {
    
    return dot(vec3(0.2126,0.7152,0.0722), color);
}


void main()
{
    vec2 uv = o_uv;
    color = texture(u_texture, uv);
    color.a = 1;

    vec3 luminance = vec3(computeLuminance(color.xyz));

    color.xyz = mix(luminance, color.xyz, u_saturation);

    
}
