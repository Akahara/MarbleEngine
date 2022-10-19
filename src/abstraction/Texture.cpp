#include "Texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "../Utils/Debug.h"

namespace Renderer {

Texture::Texture()
  : m_RendererID(0), m_width(0), m_height(0)
{
}

Texture::Texture(const std::string &path)
  : m_RendererID(0), m_width(0), m_height(0)
{
  stbi_set_flip_vertically_on_load(1);

  unsigned char *localBuffer = stbi_load(path.c_str(), &m_width, &m_height, nullptr, 4);

  glGenTextures(1, &m_RendererID);
  glBindTexture(GL_TEXTURE_2D, m_RendererID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if (localBuffer) {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(localBuffer);
  } else {
	std::cout << "\nError: Failed to load texture" << std::endl;
	std::cout << stbi_failure_reason() << std::endl;
	__debugbreak();
	stbi_image_free(localBuffer);
  }
}

Texture::Texture(unsigned int width, unsigned int height)
  : m_width(width), m_height(height)
{
  glGenTextures(1, &m_RendererID);
  glBindTexture(GL_TEXTURE_2D, m_RendererID);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RED, GL_BYTE, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(unsigned int rendererId, int width, int height)
  : m_RendererID(rendererId), m_width(width), m_height(height)
{
}

Texture::~Texture()
{
  destroy();
}

Texture::Texture(Texture &&moved) noexcept
{
  m_RendererID = moved.m_RendererID;
  m_width = moved.m_width;
  m_height = moved.m_height;
  moved.m_RendererID = 0;
  moved.m_width = 0;
  moved.m_height = 0;
}

Texture &Texture::operator=(Texture &&moved) noexcept
{
  destroy();
  new (this) Texture(std::move(moved));
  return *this;
}

void Texture::destroy()
{
  glDeleteTextures(1, &m_RendererID);
  m_RendererID = 0;
}

void Texture::bind(unsigned int slot /* = 0*/) const
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, m_RendererID);

}
void Texture::unbind(unsigned int slot /* = 0*/)
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::changeColor(uint32_t color)
{
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWidth(), getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);
}

Texture Texture::createTextureFromData(const float *data, int width, int height, int floatPerPixel)
{
  unsigned int rendererId;
  glGenTextures(1, &rendererId);
  glBindTexture(GL_TEXTURE_2D, rendererId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

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

Texture Texture::createDepthTexture(int width, int height)
{
  unsigned int rendererId;
  glGenTextures(1, &rendererId);
  glBindTexture(GL_TEXTURE_2D, rendererId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  return Texture(rendererId, width, height);
}

// TODO move this elsewhere ?
void Texture::writeToFile(const Texture &texture, const std::filesystem::path &path, bool isDepthTexture)
{
  int w, h;
  int lod = 0;
  glBindTexture(GL_TEXTURE_2D, texture.m_RendererID);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, lod, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, lod, GL_TEXTURE_HEIGHT, &h);
  char *data = new char[(size_t)w * h * 4];
  bool success;
  std::string pathStr = path.string();
  if (isDepthTexture) {
	glGetTexImage(GL_TEXTURE_2D, lod, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);
	success = stbi_write_png(pathStr.c_str(), w, h, 1, data, w); // TODO the depth texture cannot be saved without being linearized
	// and also depth-linearized (.1-1000. -> 0-1)
  } else {
	glGetTexImage(GL_TEXTURE_2D, lod, GL_RGBA, GL_UNSIGNED_BYTE, data);
	success = stbi_write_png(pathStr.c_str(), w, h, 4, data, w * 4);
  }
  delete[] data;
  if (!success)
	throw std::exception("Could not write to file");
}

}