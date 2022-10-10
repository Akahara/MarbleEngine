#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

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
	
	void Shader::SetUniformMat4x3f(std::string_view name, const glm::mat4x3 &matrix) {
	  glUniformMatrix4x3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::SetUniform1iv(std::string_view name, unsigned int count, const GLint* data) {
		 glUniform1iv(GetUniformLocation(name), count, data);
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
	  CollectTestUniforms(shader, {});
	}

	bool ShaderManager::PromptReloadAndUI()
	{
	  if (!ImGui::CollapsingHeader("Shaders"))
		return false;
	  bool needsUpdate = ImGui::Button("Reload shaders");
	  if (needsUpdate)
		ReloadShaders();
	  for (TestUniform &uniform : m_testUniforms)
		uniform.RenderImGui();
	  return needsUpdate;
	}

	void ShaderManager::ReloadShaders()
	{
	  std::vector<TestUniform> oldUniforms = std::move(m_testUniforms); // clear uniforms

	  for (ManagedShader &m : m_managedShaders) {
		*m.shader = LoadShaderFromFiles(m.vertexPath, m.fragmentPath);
		CollectTestUniforms(m.shader, oldUniforms);
	  }
	}

	static void RestorePreviousValue(TestUniform &uniform, const std::vector<TestUniform> &previousUniforms)
	{
	  for (const TestUniform &u : previousUniforms) {
		if (u.GetName() == uniform.GetName()) {
		  switch (uniform.GetSize()) {
		  case 1: uniform.SetValue<1>(u.GetValue()); break;
		  case 2: uniform.SetValue<2>(u.GetValue()); break;
		  case 3: uniform.SetValue<3>(u.GetValue()); break;
		  case 4: uniform.SetValue<4>(u.GetValue()); break;
		  }
		  break;
		}
	  }
	}

	void ShaderManager::CollectTestUniforms(Shader *shader, const std::vector<TestUniform> &previousUniforms)
	{
	  // scan for "t_*" uniforms
	  constexpr size_t bufSize = 32;
	  char uniformName[bufSize];
	  int uniformNameLength;
	  int uniformSize; // size of an uniform array (1 if non-array type)
	  GLenum uniformType;
	  int uniformsCount;
	  glGetProgramiv(shader->getId(), GL_ACTIVE_UNIFORMS, &uniformsCount);
	  for (int i = 0; i < uniformsCount; i++) {
		glGetActiveUniform(shader->getId(), (GLuint)i, bufSize, &uniformNameLength, &uniformSize, &uniformType, uniformName);
		if (strstr(uniformName, "t_") == uniformName && uniformSize == 1) {
		  switch (uniformType) {
		  case GL_FLOAT:      RestorePreviousValue(m_testUniforms.emplace_back(shader, uniformName, 1), previousUniforms); break;
		  case GL_FLOAT_VEC2: RestorePreviousValue(m_testUniforms.emplace_back(shader, uniformName, 2), previousUniforms); break;
		  case GL_FLOAT_VEC3: RestorePreviousValue(m_testUniforms.emplace_back(shader, uniformName, 3), previousUniforms); break;
		  case GL_FLOAT_VEC4: RestorePreviousValue(m_testUniforms.emplace_back(shader, uniformName, 4), previousUniforms); break;
		  }
		}
	  }
	}

	void TestUniform::RenderImGui()
	{
	  switch (m_size) {
	  case 1: if (ImGui::DragFloat (m_name.c_str(), m_value, m_speed)) SendUniformValue(); break;
	  case 2: if (ImGui::DragFloat2(m_name.c_str(), m_value, m_speed)) SendUniformValue(); break;
	  case 3: if (ImGui::DragFloat3(m_name.c_str(), m_value, m_speed)) SendUniformValue(); break;
	  case 4: if (ImGui::DragFloat4(m_name.c_str(), m_value, m_speed)) SendUniformValue(); break;
	  }
	}

	void TestUniform::SendUniformValue()
	{
	  switch (m_size) {
	  case 1: m_shader->Bind(); m_shader->SetUniform1f(m_name, m_value[0]); break;
	  case 2: m_shader->Bind(); m_shader->SetUniform2f(m_name, m_value[0], m_value[1]); break;
	  case 3: m_shader->Bind(); m_shader->SetUniform3f(m_name, m_value[0], m_value[1], m_value[2]); break;
	  case 4: m_shader->Bind(); m_shader->SetUniform4f(m_name, m_value[0], m_value[1], m_value[2], m_value[3]); break;
	  }
	}

};
