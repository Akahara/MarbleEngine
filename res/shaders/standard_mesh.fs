#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

uniform sampler2D u_Textures2D[8];

uniform float u_Strength = 1.25f;
uniform vec3 u_SunPos = vec3(1000.f);
uniform int  u_RenderChunks = 0;
uniform vec2 u_grassSteepness = vec2(0.015f);
uniform vec3 u_fogColor = vec3(.71, .86, 1.);
uniform vec3 u_fogDamping = vec3(.001, .001, .001);
uniform bool u_castShadows = false;

uniform sampler2D u_shadowMap;       // the depth map, "from the sun's point of view"
uniform vec2 u_shadowMapOrthoZRange; // = (zNear, zFar) in the orthographic projection that generated the shadow map
uniform mat4x3 u_shadowMapProj;      // a projection from world coordinates to shadow map coordinates (xy are UVs and z is the distance to the sun)

uniform vec3 u_cameraPos;

float unnormalizeOrthoDepth(float depth) {
    return depth*(u_shadowMapOrthoZRange.y-u_shadowMapOrthoZRange.x) + u_shadowMapOrthoZRange.x;
}

void main()
{
    if (u_RenderChunks==0) {
        vec4 rockSample = texture(u_Textures2D[0], o_uv);
        vec4 grassSample = texture(u_Textures2D[1], o_uv);
        color = mix(rockSample, grassSample, smoothstep(u_grassSteepness.x, u_grassSteepness.y, o_normal.y));
    } else {
        color = vec4(o_color, 1.f);
    }

    // normal only (works well with eroded terrain)
    //color.rgb = mix(vec3(0.282, 0.294, 0.294), vec3(0.894, 0.824, 0.667), dot(normalize(o_normal), normalize(u_SunPos)));


    float sunLight = 1;
    // direct sun light
    sunLight *= max(0, dot(normalize(o_normal), normalize(u_SunPos)));
    // cast shadows, see TestShadows
    if(u_castShadows) {
        vec3 shadowMapPos = u_shadowMapProj * vec4(o_pos, 1);
        if(abs(shadowMapPos.x-.5) > .5 || abs(shadowMapPos.y-.5) > .5 || shadowMapPos.z > u_shadowMapOrthoZRange.y) {
            color = color * .2 + .8 * vec4(1., .5, .5, 1);
        } else {
            float closestDistanceToSun = unnormalizeOrthoDepth(texture(u_shadowMap, shadowMapPos.xy).x);
            sunLight *= smoothstep(-0.1, 0., closestDistanceToSun - shadowMapPos.z);
        }
    }
    // apply shadows
    color.rgb *= mix(vec3(0.09,0.09,0.09), vec3(1., 1., .8), max(.1, sunLight*u_Strength));

    // apply distance fog
    color.rgb = mix(u_fogColor, color.rgb, exp(-length(o_pos - u_cameraPos)*u_fogDamping));
}