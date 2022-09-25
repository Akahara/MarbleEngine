#include "Cubemap.h"

#include <array>

#include "stb_image.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "VertexArray.h"
#include "IndexBufferObject.h"

namespace Renderer {

Cubemap::Cubemap(const std::string &front, const std::string &back, const std::string &left, const std::string &right, const std::string &top, const std::string &bottom)
{
  stbi_set_flip_vertically_on_load(0);

  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

  std::array<const std::string *, 6> files{ &front, &back, &top, &bottom, &right, &left};

  int width, height, nrChannels;
  for (unsigned int i = 0; i < 6; i++) {
    unsigned char *data = stbi_load(files[i]->c_str(), &width, &height, &nrChannels, 0);
    if (data == nullptr)
      throw std::exception("Could not load a cubemap");
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Cubemap::~Cubemap()
{
  glDeleteTextures(1, &m_id);
}

void Cubemap::Bind() const
{
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void Cubemap::Unbind()
{
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}



namespace CubemapRenderer {

static struct {
  Shader *shader;
  VertexArray *vao;
} cmRenderData;

static struct {
  VertexBufferObject *vbo;
  IndexBufferObject *ibo;
} keepAliveResources;

void Init()
{
  cmRenderData.shader = new Shader{ 
R"glsl(
#version 330 core

layout(location = 0) in vec3 i_position;

out vec3 v_texCoords;

uniform mat4 u_VP;
uniform vec3 u_displacement;

void main()
{
  v_texCoords = i_position;
  gl_Position = u_VP * vec4(i_position + u_displacement, +1.0);
}
)glsl", R"glsl(
#version 330 core

out vec4 color;

in vec3 v_texCoords;

uniform samplerCube u_skybox;

void main()
{    
  color = texture(u_skybox, v_texCoords);
}
)glsl" };

  float vertices[] = {
    -.5f, -.5f, -.5f,
    +.5f, -.5f, -.5f,
    +.5f, +.5f, -.5f,
    -.5f, +.5f, -.5f,
    -.5f, -.5f, +.5f,
    +.5f, -.5f, +.5f,
    +.5f, +.5f, +.5f,
    -.5f, +.5f, +.5f,
  };

  unsigned int indices[] = {
    0, 1, 3, 3, 1, 2,
    1, 5, 2, 2, 5, 6,
    5, 4, 6, 6, 4, 7,
    4, 0, 7, 7, 0, 3,
    3, 2, 7, 7, 2, 6,
    4, 5, 0, 0, 5, 1
  };

  keepAliveResources.vbo = new VertexBufferObject(vertices, sizeof(vertices));
  keepAliveResources.ibo = new IndexBufferObject(indices, sizeof(indices) / sizeof(indices[0]));

  VertexBufferLayout layout;
  layout.push<float>(3);
  cmRenderData.vao = new VertexArray;
  cmRenderData.vao->addBuffer(*keepAliveResources.vbo, layout);
  cmRenderData.vao->Unbind();
}

void DrawCubemap(const Cubemap &cubemap, const Camera &camera, const glm::vec3 &offset)
{
  cmRenderData.shader->Bind();
  cmRenderData.vao->Bind();
  cmRenderData.shader->SetUniformMat4f("u_VP", camera.getViewProjectionMatrix());
  cmRenderData.shader->SetUniform3f("u_displacement", offset.x, offset.y, offset.z);
  keepAliveResources.ibo->Bind();
  cubemap.Bind();

  glDepthMask(false); // do not write to depth buffer
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
  glDepthMask(true);
}

void Shutdown()
{
  delete cmRenderData.shader;
  delete cmRenderData.vao;
  delete keepAliveResources.ibo;
  delete keepAliveResources.vbo;
}

} // !namespace CubemapRenderer

} // !namespace Renderer