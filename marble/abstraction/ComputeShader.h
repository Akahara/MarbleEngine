#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <unordered_map>
namespace Renderer {

/* Immediate wrapper of the GL concept */
class ComputeShader {
private:
    unsigned int m_shaderID;
    unsigned int m_outTexture;
    glm::uvec2 m_workSize;

    std::unordered_map<std::string, int> m_uniformLocationCache;

public:
  ComputeShader(const char *path, glm::uvec2 workSize);

  ~ComputeShader();

  void use() const;
  // Send a compute shader call
  void dispatch() const;
  // Stalls untill the compute shader call finishes
  void wait() const;




  void bindImage(unsigned int texId);
  void setValues(float *values);
  std::vector<float> getValues() const;



  // uniforms

  void setUniform1i(const std::string& name, int value);
  void setUniform1f(const std::string& name, float value);
  void setUniform2f(const std::string& name, float v1, float v2);
  void setUniform3f(const std::string& name, float v1, float v2, float v3);
  void setUniform3fv(const std::string& name, unsigned int count, const float* data);
  void setUniform4f(const std::string& name, float v1, float v2, float v3, float v4);
  void setUniformMat2f(const std::string& name, const glm::mat2& matrix);
  void setUniformMat4f(const std::string& name, const glm::mat4& matrix);
  void setUniformMat4x3f(const std::string& name, const glm::mat4x3& matrix);
  void setUniform2f(const std::string& name, glm::vec2 v) { setUniform2f(name, v.x, v.y); }
  void setUniform3f(const std::string& name, glm::vec3 v) { setUniform3f(name, v.x, v.y, v.z); }
  void setUniform4f(const std::string& name, glm::vec4 v) { setUniform4f(name, v.x, v.y, v.z, v.w); }
  void setUniform1iv(const std::string& name, unsigned int count, const int* data);

private:
  int getUniformLocation(const std::string& name);

};


}