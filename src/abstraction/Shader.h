#pragma once

#include<glad/glad.h>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer {

std::string get_file_contents(const char* filename);

class Shader
{
private:

	unsigned int m_ShaderID;
	std::unordered_map<std::string, int> m_UniformLocationCache;

public:

	Shader() : m_ShaderID(0) {}
	Shader(const std::string& str_vertexShader, const std::string& str_fragmentShader);

	void Bind()const;
	void Unbind()const;
	void Delete();

	void IsProgram() const;

	void  SetUniform1i(const std::string& name, int value);
	void  SetUniform1f(const std::string& name, float value);
	void  SetUniform4f(const std::string& name, float v1, float v2, float v3, float v4);
	void  SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
	void  SetUniform1iv(const std::string& name, unsigned int count, const GLint* data);

	inline unsigned int getId() { return m_ShaderID; }

private:
	int GetUniformLocation(const std::string& name);



};

}
