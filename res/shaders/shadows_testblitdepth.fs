#version 330 core

out vec4 color;

in vec2 o_uv;

uniform sampler2D u_texture;
uniform float u_zNear = .1, u_zFar = 1000.;

// works only for orthographic projection
float unlinearize_depth(float original_depth) {
    float near = u_zNear;
    float far = u_zFar;
    return original_depth * (far - near) + near;
}

float linearize_depth(float d)
{
    return ((1.f/d) - (1.F/u_zNear))/((1.F/u_zFar)-(1.F/u_zNear));
}

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * u_zNear * u_zFar) / (u_zFar + u_zNear - z * (u_zFar - u_zNear));	
}

void main()
{             
    
    float depth = LinearizeDepth(texture(u_texture,o_uv).r) / u_zFar; // divide by far for demonstration
    color = vec4(vec3(depth), 1.0);
}