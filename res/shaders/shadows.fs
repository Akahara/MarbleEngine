#version 330 core

out vec4 color;

in vec3 v_normal;
in vec3 v_vertex;

in vec3 v_position;

uniform vec3 u_L, u_R;
uniform vec2 u_Ps;
uniform sampler2D u_shadowMap;

float rand(float s) {
  return fract(sin(s*12.9898f) * 43758.5453f);
}

vec3 hsl2rgb( in vec3 c )
{
  vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
  return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

void main()
{
    float f = floor(dot(v_normal, v_vertex * vec3(1.1, 1.32, 1.65))*100.)/100.;
    float r = rand(f);
    color = vec4(hsl2rgb(vec3(r, 1, .5)), 1);
    
    float su = dot(v_position, u_L);
    float sv = dot(v_position, u_R);
    vec2 suv = vec2(su, sv) - u_Ps;
    color.xy = suv;
    if(abs(suv.x) < .1 || abs(suv.y) < .1)
        color.rgb = vec3(1);
    else if(abs(suv.x) > 1. || abs(suv.y) > 1.)
        color.rgb = vec3(1., 0., 0.);
    else
        color = texture(u_shadowMap, suv.yx*.5+.5);
}
