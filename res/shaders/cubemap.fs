#version 330 core

out vec4 color;

in vec3 v_texCoords;

uniform samplerCube u_skybox;

void main()
{    
  color = texture(u_skybox, v_texCoords);
}