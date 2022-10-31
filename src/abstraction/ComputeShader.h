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

		GLuint m_outTexture;

		glm::uvec2 m_workSize;


	public:
		ComputeShader() : m_shaderID(0) , m_outTexture(0)
		{
			generateOutputTexture(glm::ivec2(1,1));
		}

		~ComputeShader() {
			destroy();
		}

		ComputeShader(const std::string& str_computeshader, const glm::uvec2& workSize = glm::vec2(1, 1)) 
			:	m_workSize(workSize)
		{

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

			generateOutputTexture(workSize);



		}

		void generateOutputTexture(const glm::vec2& size) {

			// generate texture
			glGenTextures(1, &m_outTexture);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_outTexture);
			// ???
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			// create empty texture
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, size.x, size.y, 0, GL_RED, GL_FLOAT, NULL);
			glBindImageTexture(0, m_outTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);


		}


		void use() {
			glUseProgram(m_shaderID);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_outTexture);
		}
		void dispatch() {
			// just keep it simple, 2d work group
			glDispatchCompute((GLuint)m_workSize.x, (GLuint)m_workSize.y, 1);
		}
		void wait() {
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		}

		static void unbind() {
			glUseProgram(0);
		}

		void		destroy() {
			glDeleteProgram(m_shaderID);
		}

		void set_values(float* values) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, (GLuint)m_workSize.x, (GLuint)m_workSize.y, 0, GL_RED, GL_FLOAT, values);
		}

		std::vector<float> get_values() {
			unsigned int collection_size = (GLuint)m_workSize.x * (GLuint)m_workSize.y;
			std::vector<float> compute_data(collection_size);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, compute_data.data());

			return compute_data;
		}

		// Unsafe
		inline unsigned int getId() { return m_shaderID; }


	};


	

}
