#version 330 core

out vec4 color;

in vec2 o_uv;

uniform sampler2D u_mask;

uniform sampler2D u_texture;

uniform vec3 u_sunPos=vec3(10,10,10);
uniform vec3 u_camForward= vec3(10,10,10);
uniform vec3 u_camPos= vec3(10,10,10);


uniform vec2 u_sunScreenSpace;

void main()
{
    vec2 uv = o_uv;

    // Convert sun pos to screenspace

    // compute how much the sun is visible
    float view = dot(normalize(u_sunPos - u_camPos), normalize(u_camForward));
    vec4 mask = texture(u_mask, uv);
   // mask.a = (view+1.F)/2.F;
    color = texture(u_texture, o_uv);

    mask.a *= 0.4;
    color.rgb += mix(vec3(0), mask.rgb, mask.a * (view+1.F)/2.F);
    //color.a = 1;

}

