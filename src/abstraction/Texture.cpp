#include "Texture.h"

#include "../Utils/Debug.h"

namespace Renderer {

Texture::Texture()
  : m_RendererID(0), m_Width(0), m_Height(0)
{
}

Texture::Texture(const std::string &path)
  : m_RendererID(0), m_Width(0), m_Height(0)
{
  stbi_set_flip_vertically_on_load(1);

  unsigned char *localBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, nullptr, 4);

  glGenTextures(1, &m_RendererID);
  glBindTexture(GL_TEXTURE_2D, m_RendererID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if (localBuffer) {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(localBuffer);
  } else {
	std::cout << "\nError: Failed to load texture" << std::endl;
	std::cout << stbi_failure_reason() << std::endl;
	__debugbreak();
	stbi_image_free(localBuffer);
  }
}

Texture::Texture(unsigned int rendererId, int width, int height)
  : m_RendererID(rendererId), m_Width(width), m_Height(height)
{
}

Texture::~Texture()
{
  Delete();
}

Texture::Texture(Texture &&moved) noexcept
{
  m_RendererID = moved.m_RendererID;
  m_Width = moved.m_Width;
  m_Height = moved.m_Height;
  moved.m_RendererID = 0;
  moved.m_Width = 0;
  moved.m_Height = 0;
}

Texture &Texture::operator=(Texture &&moved) noexcept
{
  Delete();
  new (this) Texture(std::move(moved));
  return *this;
}

void Texture::Delete()
{
  glDeleteTextures(1, &m_RendererID);
  m_RendererID = 0;
}

void Texture::Bind(unsigned int slot /* = 0*/) const
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, m_RendererID);

}
void Texture::Unbind() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::ChangeColor(uint32_t color)
{
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GetWidth(), GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
}

Texture Texture::createTextureFromData(const float *data, int width, int height, int floatPerPixel)
{
  unsigned int rendererId;
  glGenTextures(1, &rendererId);
  glBindTexture(GL_TEXTURE_2D, rendererId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GLuint dataFormat;
  switch (floatPerPixel) {
  case 1: dataFormat = GL_RED;  break;
  case 2: dataFormat = GL_RG;  	break;
  case 3: dataFormat = GL_RGB;  break;
  case 4: dataFormat = GL_RGBA; break;
  default: assert(false);
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, dataFormat, GL_FLOAT, data);
  glBindTexture(GL_TEXTURE_2D, 0);

  return Texture(rendererId, width, height);
}

}