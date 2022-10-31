#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <string_view>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace Renderer {

	class ComputeShader
	{
	private:
		unsigned int m_shaderID;
		std::unordered_map<std::string_view, int> m_uniformLocationCache;


	public:
		ComputeShader() : m_shaderID(0) {}
		ComputeShader(const std::string& str_computeshader) {

			const char* computeSource = str_computeshader.c_str();

			GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
			glShaderSource(computeShader, 1, &computeSource, NULL);
			glCompileShader(computeShader);

			m_shaderID = glCreateProgram();

			glAttachShader(m_shaderID, computeShader);
			glLinkProgram(m_shaderID);

			char infoLog[2048];
			GLsizei infoLen;
			glGetProgramInfoLog(m_shaderID, sizeof(infoLog), &infoLen, infoLog);
			if (infoLen != 0)
				std::cout << infoLog << std::endl;

			glDeleteShader(computeShader);

		}

		ComputeShader(ComputeShader&& moved) noexcept

		{
			m_shaderID = moved.m_shaderID;
			m_uniformLocationCache = std::move(moved.m_uniformLocationCache);
			moved.m_shaderID = 0;
		}


		ComputeShader& operator=(ComputeShader&& moved) noexcept
		{
			destroy();
			new (this) ComputeShader(std::move(moved));
			return *this;
		}

		ComputeShader& operator=(const ComputeShader&) = delete;

		ComputeShader(const ComputeShader&) = delete;



		void		bind() const {
			glUseProgram(m_shaderID);
		}
		static void unbind() {
			glUseProgram(0);
		}


		void		destroy() {
			glDeleteProgram(m_shaderID);
		}

		// Unsafe
		inline unsigned int getId() { return m_shaderID; }

	};


	

}
