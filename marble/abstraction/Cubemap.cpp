#include "Cubemap.h"

#include <array>

#include <stb/stb_image.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Shader.h"
#include "VertexArray.h"
#include "IndexBufferObject.h"
#include "UnifiedRenderer.h"

namespace Renderer {

Cubemap::Cubemap(const std::string &front, const std::string &back, const std::string &left, const std::string &right, const std::string &top, const std::string &bottom)
{
  stbi_set_flip_vertically_on_load(0);

  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

  std::array<const std::string *, 6> files{ &front, &back, &top, &bottom, &right, &left };

  int width, height, nrChannels;
  for (unsigned int i = 0; i < 6; i++) {
    unsigned char *data = stbi_load(files[i]->c_str(), &width, &height, &nrChannels, 0);
    if (data == nullptr)
      throw std::runtime_error("Could not load a cubemap");
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

void Cubemap::bind() const
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
}

void Cubemap::unbind()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

} // !namespace Renderer
