#version 330 core

out vec4 FragColor;

in vec2 o_uv;

uniform sampler2D u_texture;
uniform float u_exposure = 1.0f;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(u_texture, o_uv).rgb;
  
    // reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * u_exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);

} 