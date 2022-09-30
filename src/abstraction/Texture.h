#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include "stb_image.h"
#include "stb_image_write.h"

namespace Renderer {

class Texture {
private:
  unsigned int   m_RendererID;
  int m_Width, m_Height;
public:
  Texture();
  explicit Texture(const std::string &path);
  ~Texture();
  Texture(Texture &&moved) noexcept;
  Texture &operator=(Texture &&moved) noexcept;
  Texture &operator=(const Texture &) = delete;
  Texture(const Texture &) = delete;

  void Bind(unsigned int slot = 0) const;
  void Unbind() const;
  void Delete();

  void ChangeColor(uint32_t color);

  inline int GetWidth() const { return m_Width; }
  inline int GetHeight() const { return m_Height; }
  inline unsigned int getId() const { return m_RendererID; } // unsafe

  static Texture createTextureFromData(const float *data, int width, int height, int floatPerPixel = 4);
private:
  Texture(unsigned int rendererId, int width, int height);
};

}

