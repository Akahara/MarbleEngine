#include "ComputeShader.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <glad/glad.h>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool s_warning = false;

namespace Renderer {

ComputeShader::ComputeShader(const char *path, glm::uvec2 workSize)
{
	if (!s_warning) {

		int max_compute_work_group_count[3];
		int max_compute_work_group_size[3];
		int max_compute_work_group_invocations;

		for (int idx = 0; idx < 3; idx++) {
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
		}
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

		std::cout << "OpenGL Limitations: " << std::endl;
		std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
		std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
		std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

		std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
		std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
		std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;
		s_warning = false;
	}
	

  m_workSize = workSize;

  // read in shader code
  std::string computeCode;
  std::ifstream file;

  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    file.open(path);
    std::stringstream file_stream;
    file_stream << file.rdbuf();
    file.close();

    computeCode = file_stream.str();
  } catch (std::ifstream::failure e) {
    std::cerr << "failed to read compute shader file" << std::endl;
  }

  const char *computeSource = computeCode.c_str();

  // compile shader

  unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
  glShaderSource(computeShader, 1, &computeSource, NULL);
  glCompileShader(computeShader);

  // create program
  m_shaderID = glCreateProgram();
  glAttachShader(m_shaderID, computeShader);
  glLinkProgram(m_shaderID);

  // cleanup
  glDeleteShader(computeShader);

  // create input/output textures
  glGenTextures(1, &m_outTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_outTexture);

  // turns out we need this. huh.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // create empty texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, workSize.x, workSize.y, 0, GL_RED, GL_FLOAT, NULL);
  glBindImageTexture(0, m_outTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
}

ComputeShader::~ComputeShader()
{
  glDeleteProgram(m_shaderID);
}

void ComputeShader::use() const
{
  glUseProgram(m_shaderID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_outTexture);
}

void ComputeShader::dispatch() const
{
  // just keep it simple, 2d work group
  glDispatchCompute(m_workSize.x, m_workSize.y, 1);
}

void ComputeShader::bindImage(unsigned int texId) {
    glBindImageTexture(0, texId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void ComputeShader::wait() const
{
  glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void ComputeShader::setValues(float *values)
{
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_workSize.x, m_workSize.y, 0, GL_RED, GL_FLOAT, values);
}

std::vector<float> ComputeShader::getValues() const
{
  unsigned int collectionSize = m_workSize.x * m_workSize.y;
  std::vector<float> computeData(collectionSize);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, computeData.data());

  return computeData;
}






void ComputeShader::setUniform1i(const std::string& name, int value) {
	glUniform1i(getUniformLocation(name), value);
}

void ComputeShader::setUniform1f(const std::string& name, float value) {
	glUniform1f(getUniformLocation(name), value);
}

void ComputeShader::setUniform2f(const std::string& name, float v1, float v2)
{
	glUniform2f(getUniformLocation(name), v1, v2);
}

void ComputeShader::setUniform3f(const std::string& name, float v1, float v2, float v3)
{
	glUniform3f(getUniformLocation(name), v1, v2, v3);
}

void ComputeShader::setUniform3fv(const std::string& name, unsigned int count, const float* data)
{
	glUniform3fv(getUniformLocation(name), count, data);
}

void ComputeShader::setUniform4f(const std::string& name, float v1, float v2, float v3, float v4) {
	glUniform4f(getUniformLocation(name), v1, v2, v3, v4);
}

void ComputeShader::setUniformMat4f(const std::string& name, const glm::mat4& matrix) {
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void ComputeShader::setUniformMat2f(const std::string& name, const glm::mat2& matrix) {
	glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void ComputeShader::setUniformMat4x3f(const std::string& name, const glm::mat4x3& matrix) {
	glUniformMatrix4x3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void ComputeShader::setUniform1iv(const std::string& name, unsigned int count, const int* data) {
	glUniform1iv(getUniformLocation(name), count, data);
}

int ComputeShader::getUniformLocation(const std::string& name) {
	if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
		return m_uniformLocationCache[name];

	int location = glGetUniformLocation(m_shaderID, name.data());
	if (location == -1)
		std::cout << "Warning : uniform \"" << name << "\" doesn't exist ! " << std::endl;
	m_uniformLocationCache[name] = location;

	return location;
}


}