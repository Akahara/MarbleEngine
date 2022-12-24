#include "ComputeShader.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>

namespace Renderer {

ComputeShader::ComputeShader(const char *path, glm::uvec2 workSize)
{
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

}