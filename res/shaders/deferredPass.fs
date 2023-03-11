#version 330 core

layout (location=0) out vec4 color;

in vec2 o_uv;

uniform sampler2D[16] u_gBufferTextures;
uniform sampler2D u_ssaoTexture;
uniform vec3 u_cameraPos;

// Somehow make a factory out of this

uniform vec3 u_SunPos = vec3(1000,1000,1000);



// Lights structure
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

const int NR_LIGHTS = 32;
uniform PointLight u_lights[NR_LIGHTS];
uniform int u_numberOfLights = 0;

float computeSunlight(vec3 normal) {
    return max(0, dot(normalize(normal), normalize(u_SunPos)));
}

void main() 
{
    
    vec3 diffuse = texture(u_gBufferTextures[0], o_uv).rgb;
    float empty  = texture(u_gBufferTextures[0], o_uv).a;
    vec3 normal = texture(u_gBufferTextures[1], o_uv).rgb;
    vec3 fragPos = texture(u_gBufferTextures[2], o_uv).rgb;
    float depth = texture(u_gBufferTextures[5], o_uv).r;
    float occlusion = texture(u_ssaoTexture, o_uv).r;
    
    vec3 lighting  = vec3(0.3 * diffuse * occlusion);


    vec3 viewDir  = normalize(u_cameraPos - fragPos);
    for(int i = 0; i < u_numberOfLights; ++i)
    {
        if (u_lights[i].on == 0) continue;

        // diffuse
        vec3 lightDir = normalize(u_lights[i].position - fragPos);
        vec3 lightDiffuse = max(dot(normal, lightDir), 0.0) * (u_lights[i].ambient);
        
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
        vec3 specular = u_lights[i].specular;
        
        // attenuation
        float distance = length(u_lights[i].position - fragPos);
        float attenuation = 1.0 / (1.0 + u_lights[i].linear * distance + u_lights[i].quadratic * distance * distance);
        lightDiffuse *= attenuation;
        specular *= attenuation;
        lighting += lightDiffuse + specular;        
    }

    color = vec4(lighting, empty);

    // Sun coloration
    color.rgb *= mix(vec3(0.09,0.09,0.09), color.rgb, max(.1, computeSunlight(normal)*1.25f));

}