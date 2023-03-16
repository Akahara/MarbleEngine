#pragma once

#include <string>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <filesystem>
#include <array>
#include <memory>

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Texture.h"

namespace Renderer {

namespace fs = std::filesystem;

/* Immediate wrapper of the GL concept */
class Shader
{
private:
	unsigned int m_shaderID;
	std::unordered_map<std::string, int> m_uniformLocationCache;

public:
	Shader() : m_shaderID(0) {}
	Shader(const std::string& str_vertexShader, const std::string& str_fragmentShader);
	Shader(Shader &&moved) noexcept;
	Shader &operator=(Shader &&moved) noexcept;
	Shader &operator=(const Shader &) = delete;
	Shader(const Shader &) = delete;

	void bind() const;
	static void unbind();
	void destroy();

	void setUniform1i(const std::string & name, int value);
	void setUniform1f(const std::string & name, float value);
	void setUniform2f(const std::string & name, float v1, float v2);
	void setUniform3f(const std::string & name, float v1, float v2, float v3);
	void setUniform3fv(const std::string& name, unsigned int count, const float* data);
	void setUniform4f(const std::string & name, float v1, float v2, float v3, float v4);
	void setUniformMat2f(const std::string & name, const glm::mat2 &matrix);
	void setUniformMat4f(const std::string & name, const glm::mat4 &matrix);
	void setUniformMat4x3f(const std::string & name, const glm::mat4x3 &matrix);
	void setUniform2f(const std::string & name, glm::vec2 v) { setUniform2f(name, v.x, v.y); }
	void setUniform3f(const std::string & name, glm::vec3 v) { setUniform3f(name, v.x, v.y, v.z); }
	void setUniform4f(const std::string & name, glm::vec4 v) { setUniform4f(name, v.x, v.y, v.z, v.w); }
	void setUniform1iv(const std::string & name, unsigned int count, const int* data);
	// Unsafe
	inline unsigned int getId() { return m_shaderID; }

private:
	int getUniformLocation(const std::string & name);

	explicit Shader(int shaderID) : m_shaderID(shaderID) {}
	friend class ShaderFactory;
};


/* ImGui interface wrapper arround a uniform */
class TestUniform {
private:
  Shader     *m_shader;
  std::string m_name;
  float       m_value[4];
  int         m_size;
  float       m_speed;
public:
  TestUniform()
	: m_shader(nullptr), m_name(), m_value(), m_size(0), m_speed(0)
  {
  }

  TestUniform(Shader *shader, const char *name, unsigned int size, float speed = .1f)
	: m_shader(shader), m_name(name), m_value(), m_size(size), m_speed(speed)
  {
	assert(1 <= size && size <= 4);
  }

  const float *getValue() const { return &m_value[0]; }
  const std::string &getName() const { return m_name; }
  int getSize() const { return m_size; }

  template<int N>
  void setValue(const float *value) {
	assert(m_size == N);
	for (int i = 0; i < N; i++)
	  m_value[i] = value[i];
	sendUniformValue();
  }

  void setValue(float f1, float f2=0.f, float f3=0.f, float f4=0.f)
  {
	m_value[0] = f1;
	m_value[1] = f2;
	m_value[2] = f3;
	m_value[3] = f4;
	sendUniformValue();
  }

  void renderImGui();

private:
  void sendUniformValue();
};

/* Factory with which to build shader programs, shader files can be added but not removed */
class ShaderFactory {
private:
  std::vector<int> m_parts;
  fs::path m_pathPrefix;

public:
  ShaderFactory() = default;
  ~ShaderFactory();
  ShaderFactory(const ShaderFactory &) = delete;
  ShaderFactory &operator=(const ShaderFactory &) = delete;

  ShaderFactory &prefix(fs::path prefix) { m_pathPrefix /= prefix; return *this; }

  ShaderFactory &addFragment(const std::string &source);
  ShaderFactory &addVertex(const std::string &source);
  ShaderFactory &addFileFragment(const fs::path &path);
  ShaderFactory &addFileVertex(const fs::path &path);

  std::shared_ptr<Shader> build() const;

private:
  void addPart(const std::string &source, int glType);
};

/**
* A blit pass is a rendering operation that takes an input texture
* (more often than not the texture contains everything that has been
* rendered untill that point) and renders a quad that takes the full
* screen. A single shader is used.
*
* Blit passes are largelly used for VFX, for example color correction
* can be done using a simple fragment shader in a blit pass.
*/
class BlitPass {
public:
  static constexpr size_t TEXTURE_SLOT_COUNT = 8;
private:
  IndexBufferObject  m_keepAliveIBO;
  VertexBufferObject m_keepAliveVBO;
  VertexArray        m_vao;

  Shader             m_shader;
  std::array<std::shared_ptr<Texture>, TEXTURE_SLOT_COUNT> m_textures;

public:
  BlitPass();
  BlitPass(const fs::path &fragmentShaderPath);
  BlitPass(const BlitPass &) = delete;
  BlitPass &operator=(const BlitPass &) = delete;

  Shader &getShader() { return m_shader; }
  void setShader(const fs::path &fs);

  void attachInputTexture(const std::shared_ptr<Texture> &texture, unsigned int slot)
  {
	m_textures[slot] = texture;
  }

  void doBlit();
};

}
