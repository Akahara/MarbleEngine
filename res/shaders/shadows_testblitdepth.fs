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

void main()
{
    vec2 uv = o_uv;

    float depth = unlinearize_depth(texture(u_texture, uv).r);
    color = vec4(vec3((depth-u_zNear) / (u_zFar-u_zNear)), 1.);
}
