#pragma once


#include "Shader.h"
#include "Renderer.h"

#define GLFW_INCLUDE_NONE
#include<glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Renderer {

	std::string get_file_contents(const char* filename)
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
			return(contents);
		}
	}

	//================== SHADER CLASS =============//

	Shader::Shader(const std::string& str_vertexShader, const std::string& str_fragmentShader) {

		const char* vertexSource = str_vertexShader.c_str();
		const char* fragmentSource = str_fragmentShader.c_str();

		GLCall(GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER));
		GLCall(glShaderSource(vertexShader, 1, &vertexSource, NULL));
		GLCall(glCompileShader(vertexShader));

		GLCall(GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
		GLCall(glShaderSource(fragmentShader, 1, &fragmentSource, NULL));
		GLCall(glCompileShader(fragmentShader));

		GLCall(m_ShaderID = glCreateProgram());
		GLCall(glAttachShader(m_ShaderID, vertexShader));
		GLCall(glAttachShader(m_ShaderID, fragmentShader));
		GLCall(glLinkProgram(m_ShaderID));

		char infoLog[2048];
		GLCall(glGetProgramInfoLog(m_ShaderID, 2048, NULL, infoLog));
		std::cout << infoLog << std::endl;

		GLCall(glDeleteShader(vertexShader));
		GLCall(glDeleteShader(fragmentShader));
	}


	void Shader::Bind() const {

		GLCall(glUseProgram(m_ShaderID));

	}

	void Shader::IsProgram() const {

		GLCall(GLenum type = glIsProgram(m_ShaderID));
		if (type == GL_TRUE) {

			std::cout << "It's a valid program" << std::endl;

		}
		else {
			std::cout << "The shader is not valid" << std::endl;

		}

	}

	void Shader::Unbind() const {

		GLCall(glUseProgram(0));

	}

	void Shader::Delete() {

		GLCall(glDeleteProgram(m_ShaderID));

	}

	void  Shader::SetUniform1i(const std::string& name, int value) {

		GLCall(glUniform1i(GetUniformLocation(name), value));

	}

	void  Shader::SetUniform1f(const std::string& name, float value) {

		GLCall(glUniform1f(GetUniformLocation(name), value));

	}

	void  Shader::SetUniform4f(const std::string& name, float v1, float v2, float v3, float v4) {

		GLCall(glUniform4f(GetUniformLocation(name), v1, v2, v3, v4));

	}

	void  Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {

		GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix)));

	}


	void  Shader::SetUniform1iv(const std::string& name, unsigned int count, const GLint* data) {

		GLCall( glUniform1iv(GetUniformLocation(name), count, data  ) );

	}

	int Shader::GetUniformLocation(const std::string& name) {


		if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
			return m_UniformLocationCache[name];


		GLCall(int location = glGetUniformLocation(m_ShaderID, name.c_str()));
		if (location == -1)
			std::cout << "Warning : uniform \"" << name << "\" doesn't exist ! " << std::endl;
		m_UniformLocationCache[name] = location;

		return location;


	}

};

