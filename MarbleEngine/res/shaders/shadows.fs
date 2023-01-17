#version 330 core

out vec4 color;

in vec3 v_normal;
in vec3 v_vertex;

in vec3 v_position;

uniform sampler2D u_shadowMap;       // the depth map, "from the sun's point of view"
uniform vec2 u_shadowMapOrthoZRange; // = (zNear, zFar) in the orthographic projection that generated the shadow map
uniform mat4x3 u_shadowMapProj;      // a projection from world coordinates to shadow map coordinates (xy are UVs and z is the distance to the sun)

float rand(float s) {
  return fract(sin(s*12.9898) * 43758.5453);
}

vec3 hsl2rgb( in vec3 c )
{
  vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
  return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

float unnormalizeOrthoDepth(float depth) {
    return depth*(u_shadowMapOrthoZRange.y-u_shadowMapOrthoZRange.x) + u_shadowMapOrthoZRange.x;
}

void main()
{
    // generate a color individual to the surface
    float f = floor(dot(v_normal, v_vertex * vec3(1.1, 1.32, 1.65))*100.)/100.;
    float r = rand(f);
    color = vec4(hsl2rgb(vec3(r, 1, .5)), 1);
    
    // depthMapPos.x in range 0..1 (depth texture u)
    // depthMapPos.y in range 0..1 (depth texture v)
    // depthMapPos.z in range zNear..zFar (which are stored in u_shadowMapOrthoZRange)
    vec3 shadowMapPos = u_shadowMapProj * vec4(v_position, 1);

    if(abs(shadowMapPos.x-.5) > .5 || abs(shadowMapPos.y-.5) > .5 || shadowMapPos.z > u_shadowMapOrthoZRange.y) {
        color = color * .2 + .8 * vec4(1., .5, .5, 1); // outside of the depth map
    } else {
        float closestDistanceToSun = unnormalizeOrthoDepth(
            texture(u_shadowMap, shadowMapPos.xy).x // sample depth
        );
        float sunLight = smoothstep(-0.01, 0., closestDistanceToSun - shadowMapPos.z);
        color *= vec4(vec3(sunLight * .8 + .2), 1);
    }
}
