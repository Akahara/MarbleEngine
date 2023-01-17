#version 330 core

#define MAX_NB_POINT_LIGHTS 12

in vec2 o_uv;
in vec3 o_normal;
in vec3 o_pos;
in vec3 o_color;

struct PointLight {

    int on; // does this work ? yes it does

    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
   
};

uniform PointLight u_lights[MAX_NB_POINT_LIGHTS];
uniform int        u_numberOfLights = 0;
uniform vec3       u_cameraPos;

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

vec4 computeExtraLights(vec3 normal) {
    vec3 viewDir = normalize(u_cameraPos - o_pos);
    vec4 extra = vec4(0);

    for (int i = 0; i < u_numberOfLights; i++) {
        PointLight light = u_lights[i];
        if (light.on == 0) continue;
        extra.rgb += CalcPointLight(light, normal, o_pos, viewDir);
    }

    return extra;
}