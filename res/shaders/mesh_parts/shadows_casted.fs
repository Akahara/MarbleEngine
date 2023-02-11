#version 330 core

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

// This tells what texture slot is the normal texture of the index-Texture
uniform int       u_NormalsTextureSlot[8]; 
uniform sampler2D u_Textures2D[8];
uniform vec3      u_SunPos;
uniform sampler2D u_shadowMap;            // the depth map, "from the sun's point of view"
uniform vec2      u_shadowMapOrthoZRange; // = (zNear, zFar) in the orthographic projection that generated the shadow map
uniform mat4x3    u_shadowMapProj;        // a projection from world coordinates to shadow map coordinates (xy are UVs and z is the distance to the sun)

float unnormalizeOrthoDepth(float depth) {
    return depth*(u_shadowMapOrthoZRange.y-u_shadowMapOrthoZRange.x) + u_shadowMapOrthoZRange.x;
}

float computeSunlight(vec3 normal) {
    if (u_NormalsTextureSlot[0] != -1) {
        vec4 sn = texture(u_Textures2D[0],o_uv); // TODO see note in normal_normalmap.fs
        normal = normal * sn.rgb;
    }

    float sunlight = max(0, dot(normalize(normal), normalize(u_SunPos)));

    vec3 shadowMapPos = u_shadowMapProj * vec4(o_pos, 1);
    if(abs(shadowMapPos.x-.5) > .5 || abs(shadowMapPos.y-.5) > .5 || shadowMapPos.z > u_shadowMapOrthoZRange.y)
        return .5; // outside of the sun viewport
    float closestDistanceToSun = unnormalizeOrthoDepth(texture(u_shadowMap, shadowMapPos.xy).x);
    sunlight *= smoothstep(-0.1, 0., closestDistanceToSun - shadowMapPos.z);

    return sunlight;
}