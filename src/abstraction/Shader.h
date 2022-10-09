#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <string_view>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
	void SetUniform1iv(std::string_view name, unsigned int count, const GLint* data);
	// Unsafe
	inline unsigned int getId() { return m_ShaderID; }
private:
	int GetUniformLocation(std::string_view name);
};


class ShaderManager {
private:
  struct ManagedShader {
	Shader *shader;
	const char *vertexPath;
	const char *fragmentPath;
  };
  std::vector<ManagedShader> m_managedShaders;
  bool                       m_needUpdate;
public:
  void AddShader(Shader *shader, const char *vertexPath, const char *fragmentPath, bool loadNow=true);

  bool PromptReload();

  void ReloadShaders();
};

}
