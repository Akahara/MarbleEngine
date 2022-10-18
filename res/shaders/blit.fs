#version 330 core

out vec4 color;

in vec2 o_uv;

uniform sampler2D u_texture;
uniform vec2 u_screenSize;

// TODO? use a vec3 for the vignette effect, to avoid doing 3 setUniform calls
uniform float u_vignetteMin = 1.2;
uniform float u_vignetteMax = .045;
uniform float u_vignetteStrength = .2;

void applyVignette(inout vec4 color) {
    float l = max(u_screenSize.x, u_screenSize.y)*.5;
    vec2 uv = gl_FragCoord.xy-u_screenSize*.5;
    color.rgb *= 1-u_vignetteStrength * smoothstep(l*l*u_vignetteMax, l*l*u_vignetteMin, dot(uv, uv));
}

void main()
{
    vec2 uv = o_uv;
    color = texture(u_texture, uv);

    applyVignette(color);

    color.a = 1;
}
