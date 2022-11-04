#version 460 core


out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;


flat in float o_texId;
in vec3 o_color;


in vec4 o_clipspace;


uniform sampler2D u_ReflectionTexture;
uniform sampler2D u_RefractionTexture;


void main() 
{
    vec2 screenspace = ((o_clipspace.xy/o_clipspace.w)+1) * 0.5;

    vec4 texReflec = texture(u_ReflectionTexture, reflect);
    vec2 reflect = vec2(screenspace.x, -screenspace.y);
    vec4 texRefrac = texture(u_RefractionTexture, screenspace);
    //color = mix(texRefrac, texRefrac, 0.5);
    color = texRefrac;
    color.a = 0.5;
}