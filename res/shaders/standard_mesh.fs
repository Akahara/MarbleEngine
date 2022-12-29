#version 330 core

out vec4 color;

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;
flat in int o_texId;

uniform sampler2D u_Textures2D[8];

uniform float u_Strength = 1.25f;
uniform vec3 u_SunPos = vec3(1000.f);
uniform int  u_RenderChunks = 0;
uniform vec2 u_grassSteepness = vec2(0.015f);
uniform vec3 u_fogColor = vec3(.71, .86, 1.);
uniform vec3 u_fogDamping = vec3(.001, .001, .001);
uniform bool u_castShadows = false;
uniform bool u_isTerrain = false; // somehow fix this, this is not a great solution but it works


// This tells what texture slot is the normal texture of the index-Texture
uniform int u_NormalsTextureSlot[8] = { 
-1, -1, -1, -1, -1, -1, -1, -1
}; 

uniform sampler2D u_shadowMap;       // the depth map, "from the sun's point of view"
uniform vec2 u_shadowMapOrthoZRange; // = (zNear, zFar) in the orthographic projection that generated the shadow map
uniform mat4x3 u_shadowMapProj;      // a projection from world coordinates to shadow map coordinates (xy are UVs and z is the distance to the sun)

uniform vec3 u_cameraPos;

/////////////////////////////////////////////////////////////
#define MAX_NB_POINT_LIGHTS 12

struct PointLight  {

    int on; // does this work ? yes it does

    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
   
};


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
uniform PointLight u_lights[MAX_NB_POINT_LIGHTS];
uniform int numberOfLights = 0;

///////////////////////////////////////////////////////////////

//#define MESA_SCENE

#ifdef MESA_SCENE
const vec3 mesa_colors[5] = vec3[5](
    vec3(0.725, 0.455, 0.102),
    vec3(0.949, 0.639, 0.369),
    vec3(0.941, 0.482, 0.412),
    vec3(0.867, 0.720, 0.698),
    vec3(0.867, 0.630, 0.325)
);
#endif

float unnormalizeOrthoDepth(float depth) {
    return depth*(u_shadowMapOrthoZRange.y-u_shadowMapOrthoZRange.x) + u_shadowMapOrthoZRange.x;
}

void main()
{
    vec3 normal = o_normal;
#ifdef MESA_SCENE
    // for the mesa scene, a simple texture sample does not sufice
    color = mix(texture(u_Textures2D[0], o_uv), vec4(mesa_colors[int(floor(o_pos.y*.5))%5], 1), .7);
#else
    // regular texture sample
    //color.rgb = mix(color.rgb, vec3(2,2,2), dot(normalize(o_normal), normalize(u_SunPos)));
    if (u_isTerrain) {
        if (u_RenderChunks==0) {
            vec4 rockSample = texture(u_Textures2D[0], o_uv);
            vec4 grassSample = texture(u_Textures2D[1], o_uv);
            //normal = texture(u_Textures2D[2], o_uv).rgb; //only for POC1
            color = mix(rockSample, grassSample, smoothstep(u_grassSteepness.x, u_grassSteepness.y, o_normal.y));
        } else {
            color = vec4(o_color, 1.f);
        } 
    }
#endif

    // normal only (works well with eroded terrain)

    if (!u_isTerrain) {
        color = texture(u_Textures2D[o_texId], o_uv);
    }
    float sunLight = 1;
    // direct sun light + normal map if defined, TODO clean this 
    
    if (u_NormalsTextureSlot[o_texId] != -1) {
        int index_normal_slot = u_NormalsTextureSlot[o_texId];
        vec4 sn = texture(u_Textures2D[index_normal_slot],o_uv);
        normal = normal * sn.rgb;
    }

    sunLight *= max(0, dot(normalize(normal), normalize(u_SunPos)));
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

    // apply lights
    vec3 viewDir = normalize(u_cameraPos - o_pos);
    vec3 lightDir = normalize(u_SunPos); // default is sun direction

    for (int i = 0; i < numberOfLights; i++) {
        PointLight light = u_lights[i];
        if (light.on == 0) continue;
        color.rgb += CalcPointLight(light, normal, o_pos, viewDir);
    }


    // apply distance fog
    color.rgb = mix(u_fogColor, color.rgb, exp(-length(o_pos - u_cameraPos)*u_fogDamping));
}


////////////////////////////////////////////////////
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient;
    vec3 diffuse  = light.diffuse; 
    vec3 specular = light.specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}