#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../vendor/imgui/imgui.h"

#include "Shader.h"
#include "Renderer.h"
#include "UnifiedRenderer.h"


namespace Renderer {

	std::string GetFileContents(const char* filename)
	{
		std::ifstream in(filename, std::ios::binary);
		if (in)
		{
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
			return contents;
		}
		throw std::exception("File not found");
	}

	//================== SHADER CLASS =============//

	Shader::Shader(const std::string& str_vertexShader, const std::string& str_fragmentShader) {

		const char* vertexSource = str_vertexShader.c_str();
		const char* fragmentSource = str_fragmentShader.c_str();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);

		m_ShaderID = glCreateProgram();
		glAttachShader(m_ShaderID, vertexShader);
		glAttachShader(m_ShaderID, fragmentShader);
		glLinkProgram(m_ShaderID);

		char infoLog[2048];
		GLsizei infoLen;
		glGetProgramInfoLog(m_ShaderID, sizeof(infoLog), &infoLen, infoLog);
		if(infoLen != 0)
			std::cout << infoLog << std::endl;

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	Shader::Shader(Shader &&moved) noexcept
	{
	  m_ShaderID = moved.m_ShaderID;
	  m_UniformLocationCache = std::move(moved.m_UniformLocationCache);
	  moved.m_ShaderID = 0;
	}

	Shader &Shader::operator=(Shader &&moved) noexcept
	{
	  Delete();
	  new (this) Shader(std::move(moved));
	  return *this;
	}

	void Shader::Bind() const {
		glUseProgram(m_ShaderID);
	}

	void Shader::Unbind() {
		glUseProgram(0);
	}

	void Shader::Delete() {
		glDeleteProgram(m_ShaderID);
	}

	void Shader::SetUniform1i(std::string_view name, int value) {
		glUniform1i(GetUniformLocation(name), value);
	}

	void Shader::SetUniform1f(std::string_view name, float value) {
		glUniform1f(GetUniformLocation(name), value);
	}

	void Shader::SetUniform2f(std::string_view name, float v1, float v2)
	{
	  glUniform2f(GetUniformLocation(name), v1, v2);
	}

	void Shader::SetUniform3f(std::string_view name, float v1, float v2, float v3)
	{
	  glUniform3f(GetUniformLocation(name), v1, v2, v3);
	}

	void Shader::SetUniform4f(std::string_view name, float v1, float v2, float v3, float v4) {
		glUniform4f(GetUniformLocation(name), v1, v2, v3, v4);
	}

	void Shader::SetUniformMat4f(std::string_view name, const glm::mat4& matrix) {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}


	void Shader::SetUniform1iv(std::string_view name, unsigned int count, const GLint* data) {
		 glUniform1iv(GetUniformLocation(name), count, data  ) ;
	}

	int Shader::GetUniformLocation(std::string_view name) {
		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
			return m_UniformLocationCache[name];

		int location = glGetUniformLocation(m_ShaderID, name.data());
		if (location == -1)
			std::cout << "Warning : uniform \"" << name << "\" doesn't exist ! " << std::endl;
		m_UniformLocationCache[name] = location;

		return location;
	}

	void ShaderManager::AddShader(Shader *shader, const char *vertexPath, const char *fragmentPath, bool loadNow)
	{
	  m_managedShaders.emplace_back(shader, vertexPath, fragmentPath);
	  if(loadNow)
	  	*shader = LoadShaderFromFiles(vertexPath, fragmentPath);
	}

	bool ShaderManager::PromptReload()
	{
	  if (ImGui::Button("Reload shaders")) {
		ReloadShaders();
		return true;
	  }
	  return false;
	}

	void ShaderManager::ReloadShaders()
	{
	  for(ManagedShader &m : m_managedShaders)
		*m.shader = LoadShaderFromFiles(m.vertexPath, m.fragmentPath);
	}

};
