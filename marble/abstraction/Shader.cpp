#include "Shader.h"

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

#include "UnifiedRenderer.h"
#include "../Utils/Debug.h"

namespace Renderer {

	//================== SHADER CLASS =============//
  
	Shader::Shader(const std::string& str_vertexShader, const std::string& str_fragmentShader)
	{
		const char* vertexSource = str_vertexShader.c_str();
		const char* fragmentSource = str_fragmentShader.c_str();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexSource, NULL);
		glCompileShader(vertexShader);

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
		glCompileShader(fragmentShader);

		m_shaderID = glCreateProgram();
		glAttachShader(m_shaderID, vertexShader);
		glAttachShader(m_shaderID, fragmentShader);
		glLinkProgram(m_shaderID);

		char infoLog[2048];
		GLsizei infoLen;
		glGetProgramInfoLog(m_shaderID, sizeof(infoLog), &infoLen, infoLog);
		if (infoLen != 0) {
			std::cerr << infoLog << std::endl;
			MARBLE_DEBUGBREAK();
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	Shader::Shader(Shader &&moved) noexcept
	{
	  m_shaderID = moved.m_shaderID;
	  m_uniformLocationCache = std::move(moved.m_uniformLocationCache);
	  moved.m_shaderID = 0;
	}

	Shader &Shader::operator=(Shader &&moved) noexcept
	{
	  destroy();
	  new (this) Shader(std::move(moved));
	  return *this;
	}

	void Shader::bind() const {
		glUseProgram(m_shaderID);
	}

	void Shader::unbind() {
		glUseProgram(0);
	}

	void Shader::destroy() {
		glDeleteProgram(m_shaderID);
	}

	void Shader::setUniform1i(const std::string & name, int value) {
		glUniform1i(getUniformLocation(name), value);
	}

	void Shader::setUniform1f(const std::string & name, float value) {
		glUniform1f(getUniformLocation(name), value);
	}

	void Shader::setUniform2f(const std::string & name, float v1, float v2)
	{
	  glUniform2f(getUniformLocation(name), v1, v2);
	}

	void Shader::setUniform3f(const std::string & name, float v1, float v2, float v3)
	{
	  glUniform3f(getUniformLocation(name), v1, v2, v3);
	}

	void Shader::setUniform4f(const std::string & name, float v1, float v2, float v3, float v4) {
		glUniform4f(getUniformLocation(name), v1, v2, v3, v4);
	}
	
	void Shader::setUniformMat4f(const std::string & name, const glm::mat4& matrix) {
		glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::setUniformMat2f(const std::string & name, const glm::mat2& matrix) {
		glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}
	
	void Shader::setUniformMat4x3f(const std::string & name, const glm::mat4x3 &matrix) {
	  glUniformMatrix4x3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::setUniform1iv(const std::string & name, unsigned int count, const int* data) {
		 glUniform1iv(getUniformLocation(name), count, data);
	}

	int Shader::getUniformLocation(const std::string &name) {
		if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
			return m_uniformLocationCache[name];

		int location = glGetUniformLocation(m_shaderID, name.data());
		if (location == -1)
			std::cout <<  "Warning : uniform \"" << name << "\" doesn't exist ! " << std::endl;
		m_uniformLocationCache[name] = location;

		return location;
	}


	void TestUniform::renderImGui()
	{
	  switch (m_size) {
	  case 1: if (ImGui::DragFloat (m_name.c_str(), m_value, m_speed)) sendUniformValue(); break;
	  case 2: if (ImGui::DragFloat2(m_name.c_str(), m_value, m_speed)) sendUniformValue(); break;
	  case 3: if (ImGui::DragFloat3(m_name.c_str(), m_value, m_speed)) sendUniformValue(); break;
	  case 4: if (ImGui::DragFloat4(m_name.c_str(), m_value, m_speed)) sendUniformValue(); break;
	  }
	}

	void TestUniform::sendUniformValue()
	{
	  switch (m_size) {
	  case 1: m_shader->bind(); m_shader->setUniform1f(m_name, m_value[0]); break;
	  case 2: m_shader->bind(); m_shader->setUniform2f(m_name, m_value[0], m_value[1]); break;
	  case 3: m_shader->bind(); m_shader->setUniform3f(m_name, m_value[0], m_value[1], m_value[2]); break;
	  case 4: m_shader->bind(); m_shader->setUniform4f(m_name, m_value[0], m_value[1], m_value[2], m_value[3]); break;
	  }
	}

	std::shared_ptr<Shader> ShaderFactory::build() const
	{
	  int shaderID = glCreateProgram();
	  for(int partID : m_parts)
		glAttachShader(shaderID, partID);
	  glLinkProgram(shaderID);
	  return std::shared_ptr<Shader>(new Shader(shaderID));
	}

	void ShaderFactory::addPart(const std::string &source, int glType)
	{
	  GLuint partID = glCreateShader(glType);
	  const char *csource = source.c_str();
	  glShaderSource(partID, 1, &csource, NULL);
	  glCompileShader(partID);
	  m_parts.push_back(partID);
	}

	ShaderFactory::~ShaderFactory()
	{
	  for (int partID : m_parts)
		glDeleteShader(partID);
	}

	static std::string readFile(const fs::path &path)
	{
	  std::ifstream file{ path };
	  if (!file.good())
		throw std::runtime_error("Could not load a shader file");
	  std::stringstream buffer;
	  buffer << file.rdbuf();
	  return buffer.str();
	}

	ShaderFactory &ShaderFactory::addFragment(const std::string &source)
	{
	  addPart(source, GL_FRAGMENT_SHADER);
	  return *this;
	}

	ShaderFactory &ShaderFactory::addVertex(const std::string &source)
	{
	  addPart(source, GL_VERTEX_SHADER);
	  return *this;
	}

	ShaderFactory &ShaderFactory::addFileFragment(const fs::path &path)
	{
	  addFragment(readFile(m_pathPrefix / path));
	  return *this;
	}

	ShaderFactory &ShaderFactory::addFileVertex(const fs::path &path)
	{
	  addVertex(readFile(m_pathPrefix / path));
	  return *this;
	}

	BlitPass::BlitPass()
	  : BlitPass("res/shaders/blit.fs")
	{
	}

	BlitPass::BlitPass(const fs::path &fragmentShaderPath)
	{
	  m_shader = std::move(*loadShaderFromFiles("res/shaders/blit.vs", fragmentShaderPath));
	  m_keepAliveIBO = IndexBufferObject({ 0, 2, 1, 3, 2, 0 });
	  m_vao.addBuffer(m_keepAliveVBO, VertexBufferLayout{}, m_keepAliveIBO);
	  VertexArray::unbind();
	}

	void BlitPass::setShader(const fs::path &fs)
	{
	  m_shader = std::move(*loadShaderFromFiles("res/shaders/blit.vs", fs));
	}

	void BlitPass::doBlit()
	{
	  Renderer::clear();
	  for (unsigned int slot = 0; slot < m_textures.size(); slot++)
		if (m_textures[slot])
		  m_textures[slot]->bind(slot);
	  m_shader.bind();
	  m_vao.bind();
	  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	  m_shader.unbind();
	  VertexArray::unbind();
	}
};
