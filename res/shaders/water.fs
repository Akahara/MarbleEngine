#version 400 core


out vec4 color;

in vec2 o_uv;
in vec2 o_texCoordsWater;
in vec4 o_clipspace;
in vec3 o_sunPos;
in vec3 o_fromLightVector;


uniform sampler2D u_RefractionTexture;
uniform sampler2D u_ReflectionTexture;
uniform sampler2D u_dudvMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_depthMap;

uniform float u_moveFactor;

in vec3 o_toCameraVector;

const float waveStrength = 0.04;
const vec3 waterNormal = vec3(0.f,1.f,0.f);
const vec3 sunColor = vec3(1,1,1) ;
const float shineDamper = 20.0f;
const float reflectivity = 0.5f;


vec3 computeReflection(vec3 N, vec3 I) {

    return I - 2.0 * dot(N,I) * N;

}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec4 murkyColor = vec4(rgb2hsv(  vec3(0.0,255-77,255-44)   ), 0.0) ;

void main() 
{
    vec3 viewVector = normalize(o_toCameraVector);


    vec2 screenspace = ((o_clipspace.xy/o_clipspace.w)+1) * 0.5;


    vec2 refract = vec2(screenspace.x,  screenspace.y);
    vec2 reflect = vec2(screenspace.x,  1-screenspace.y);


    float near = 0.1f; // todo uniform ?
    float far = 1000.f;
    float depth = texture(u_depthMap, refract).r;
    float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

    depth = gl_FragCoord.z;
    float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

    float waterDepth = floorDistance - waterDistance;


    vec2 distortedTexCoords = texture(u_dudvMap, vec2(o_uv.x + u_moveFactor, o_uv.y)).rg*0.1;
	distortedTexCoords = o_uv + vec2(distortedTexCoords.x, distortedTexCoords.y+u_moveFactor);
	vec2 distortion = (texture(u_dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength * clamp(waterDepth/20.0f, 0.f, 1.f);


    refract += distortion;
    refract = clamp(refract, 0.001, 0.999);
    reflect += distortion;
    refract.x = clamp(reflect.x, 0.001, 0.999);

    vec4 texReflec = texture(u_ReflectionTexture, reflect);
    vec4 texRefrac = texture(u_RefractionTexture, refract);
    texRefrac = mix(texRefrac, murkyColor, clamp(waterDepth/5000.f, 0.0, 1.0));

    vec4 normalMapColour = texture(u_normalMap, distortedTexCoords);
    vec3 normal = vec3(normalMapColour.r * 2.0 - 1.0, normalMapColour.b * 3.0, normalMapColour.g *2.0 - 1.0);
    normal = normalize(normal);
    
    float fresnel = dot(normal, viewVector);

    // specular lighting stuff
    vec3 reflectedLight = computeReflection(normalize(o_fromLightVector), normal);
	float specular = max(dot(reflectedLight, viewVector), 0.0);
	specular = pow(specular, shineDamper);
	vec3 specularHighlights = sunColor * specular * reflectivity * clamp(waterDepth/5.0f, 0.f, 1.f);

    color = mix(texRefrac, texReflec, 1 - fresnel);


    color = mix(color, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(specularHighlights,0.0)  ;
    color.a = clamp(waterDepth/5.0f, 0.f, 1.f);
}