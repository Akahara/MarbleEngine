#pragma once

#include <iostream>
#include <string>
#include <filesystem>

#include <glm/vec2.hpp>

namespace Renderer {

class Texture {
private:
  unsigned int m_rendererID;
  int          m_width, m_height;
public:
  Texture();
  explicit Texture(const std::string &path);
  Texture(unsigned int width, unsigned int height);
  ~Texture();
  Texture(Texture &&moved) noexcept;
  Texture &operator=(Texture &&moved) noexcept;
  Texture &operator=(const Texture &other) = delete;
 // Texture& operator=(const Texture& other) 
 // { 
	//m_RendererID = other.m_RendererID;
	//m_width = other.m_width; 
	//m_height = other.m_height;
	//return *this;
 // }
  Texture(const Texture &) = delete;

  void bind(unsigned int slot = 0) const;
  static void unbind(unsigned int slot = 0);
  void destroy();

  void changeColor(uint32_t color);

  inline int getWidth() const { return m_width; }
  inline int getHeight() const { return m_height; }
  inline glm::vec2 getSize() const { return { m_width, m_height }; };
  inline unsigned int getId() const { return m_rendererID; } // unsafe

  static Texture createTextureFromData(const float *data, int width, int height, int floatPerPixel = 4);
  static Texture createDepthTexture(int width, int height);

  /* Writes a texture to a .png file, depth textures aren't supported */
  static void writeToFile(const Texture &texture, const std::filesystem::path &path);
private:
  Texture(unsigned int rendererId, int width, int height);
};

}

