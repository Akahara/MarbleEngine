#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <string_view>
#include <glm/glm.hpp>
#include <vector>

namespace Renderer {

class Shader
{
private:
	unsigned int m_shaderID;
	std::unordered_map<std::string_view, int> m_uniformLocationCache;

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

	void setUniform1i(std::string_view name, int value);
	void setUniform1f(std::string_view name, float value);
	void setUniform2f(std::string_view name, float v1, float v2);
	void setUniform3f(std::string_view name, float v1, float v2, float v3);
	void setUniform4f(std::string_view name, float v1, float v2, float v3, float v4);
	void setUniformMat4f(std::string_view name, const glm::mat4 &matrix);
	void setUniformMat4x3f(std::string_view name, const glm::mat4x3 &matrix);
	void setUniform2f(std::string_view name, glm::vec2 v) { setUniform2f(name, v.x, v.y); }
	void setUniform3f(std::string_view name, glm::vec3 v) { setUniform3f(name, v.x, v.y, v.z); }
	void setUniform4f(std::string_view name, glm::vec4 v) { setUniform4f(name, v.x, v.y, v.z, v.w); }
	void setUniform1iv(std::string_view name, unsigned int count, const GLint* data);
	// Unsafe
	inline unsigned int getId() { return m_shaderID; }
private:
	int getUniformLocation(std::string_view name);
};


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

class ShaderManager {
private:
  struct ManagedShader {
	Shader *shader;
	const char *vertexPath;
	const char *fragmentPath;
  };
  std::vector<ManagedShader> m_managedShaders;
  std::vector<TestUniform>   m_testUniforms;
public:
  void addShader(Shader *shader, const char *vertexPath, const char *fragmentPath, bool loadNow=true);

  bool promptReloadAndUI();

  void reloadShaders();
private:
  void collectTestUniforms(Shader *shader, const std::vector<TestUniform> &previousUniforms);
};

}
