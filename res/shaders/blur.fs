#version 330 core
out vec4 FragColor;

in vec2 o_uv;

uniform sampler2D u_target;
uniform sampler2D u_texture;
// really bad blur but its temp
void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_texture, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_texture, o_uv + offset).r;
        }
    }
    FragColor = vec4(vec3(result / (16.F)),1);
    result = result / 16.F;
    FragColor = texture(u_target, o_uv);

    FragColor.rgb = FragColor.rgb - vec3(1-result);
    //FragColor = vec4(1,0,0,1);
}  