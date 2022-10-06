#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;




uniform vec3 u_SunPos;
uniform float u_Strenght;

uniform sampler2D u_Texture2D;

vec3 sun_dir = normalize(u_SunPos);


void main()
{
    color = texture(u_Texture2D, o_uv);
    // .2 + dot(normal, sun) * .8
    float ambiantLight = 0.2 + dot(o_normal, sun_dir) * 0.8;

    // ambiantColor + lerp(0, sun, dot(sunDir, normal))
     color.rgb = vec3(0.09,0.09,0.09) * 0.2 + vec3(1, 1, 0.0) * dot(o_normal, sun_dir) * u_Strenght ;

}
