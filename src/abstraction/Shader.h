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
	unsigned int m_ShaderID;
	std::unordered_map<std::string_view, int> m_UniformLocationCache;

public:
	Shader() : m_ShaderID(0) {}
	Shader(const std::string& str_vertexShader, const std::string& str_fragmentShader);
	Shader(Shader &&moved) noexcept;
	Shader &operator=(Shader &&moved) noexcept;
	Shader &operator=(const Shader &) = delete;
	Shader(const Shader &) = delete;

	void Bind() const;
	static void Unbind();
	void Delete();
	void SetUniform1i(std::string_view name, int value);
	void SetUniform1f(std::string_view name, float value);
	void SetUniform2f(std::string_view name, float v1, float v2);
	void SetUniform3f(std::string_view name, float v1, float v2, float v3);
	void SetUniform4f(std::string_view name, float v1, float v2, float v3, float v4);
	void SetUniformMat4f(std::string_view name, const glm::mat4 &matrix);
	void SetUniformMat4x3f(std::string_view name, const glm::mat4x3 &matrix);
	void SetUniform2f(std::string_view name, glm::vec2 v) { SetUniform2f(name, v.x, v.y); }
	void SetUniform3f(std::string_view name, glm::vec3 v) { SetUniform3f(name, v.x, v.y, v.z); }
	void SetUniform4f(std::string_view name, glm::vec4 v) { SetUniform4f(name, v.x, v.y, v.z, v.w); }
	void SetUniform1iv(std::string_view name, unsigned int count, const GLint* data);
	// Unsafe
	inline unsigned int getId() { return m_ShaderID; }
private:
	int GetUniformLocation(std::string_view name);
};


class TestUniform {
private:
  Shader     *m_shader;
  std::string m_name;
  float       m_value[4];
  int         m_size;
  float       m_speed;
public:
  TestUniform(Shader *shader, const char *name, unsigned int size, float speed = .1f)
	: m_shader(shader), m_name(name), m_value(), m_size(size), m_speed(speed)
  {
	assert(1 <= size && size <= 4);
  }

  const float *GetValue() const { return &m_value[0]; }
  const std::string &GetName() const { return m_name; }
  int GetSize() const { return m_size; }

  template<int N>
  void SetValue(const float *value) {
	assert(m_size == N);
	for (int i = 0; i < N; i++)
	  m_value[i] = value[i];
	SendUniformValue();
  }

  void RenderImGui();

private:
  void SendUniformValue();
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
  void AddShader(Shader *shader, const char *vertexPath, const char *fragmentPath, bool loadNow=true);

  bool PromptReloadAndUI();

  void ReloadShaders();
private:
  void CollectTestUniforms(Shader *shader, const std::vector<TestUniform> &previousUniforms);
};

}
