#version 330 core
out vec4 FragColor;

in vec2 o_uv;

uniform sampler2D u_sceneTexture;

// dont forget to bind the texture
uniform sampler2D u_finalBloom;

uniform float u_exposure = 1;
uniform float u_bloomStrength = 0.05f;


vec3 bloom()
{
    vec3 hdrColor = texture(u_sceneTexture, o_uv).rgb;
    vec3 bloomColor = texture(u_finalBloom, o_uv).rgb;
    return mix(hdrColor, bloomColor, u_bloomStrength); // linear interpolation
}

void main()
{
    vec3 result = vec3(0.0);
    result = bloom();
    // tone mapping
    result = vec3(1.0) - exp(-result * u_exposure);
    // also gamma correct while we're at it
    
    //const float gamma = 2.2;

      //  result = pow(result, vec3(1.0 / gamma));

    //result.rgb = max(vec3(0), result.rgb-1);

    FragColor = vec4(result, 1.0);
}