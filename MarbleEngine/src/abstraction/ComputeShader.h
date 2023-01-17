#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace Renderer {

/* Immediate wrapper of the GL concept */
class ComputeShader {
private:
    unsigned int m_shaderID;
    unsigned int m_outTexture;
    glm::uvec2 m_workSize;

public:
  ComputeShader(const char *path, glm::uvec2 workSize);

  ~ComputeShader();

  void use() const;
  // Send a compute shader call
  void dispatch() const;
  // Stalls untill the compute shader call finishes
  void wait() const;

  void setValues(float *values);
  std::vector<float> getValues() const;
};


}