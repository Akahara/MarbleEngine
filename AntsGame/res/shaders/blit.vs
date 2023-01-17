#version 330 core

const vec2[4] vertices = vec2[4](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 1),
    vec2(1, 0)
);

out vec2 o_uv;

void main()
{
    vec2 pos = vertices[gl_VertexID];
    o_uv = pos;
    gl_Position = vec4(pos*2.-1., 0., 1.);
}
