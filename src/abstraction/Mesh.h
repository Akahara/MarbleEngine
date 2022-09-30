#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "VertexArray.h"

namespace Renderer {

struct Vertex {
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;
};

class Mesh {
private:
  Renderer::VertexBufferObject m_VBO;
  Renderer::IndexBufferObject  m_IBO;
  Renderer::VertexArray        m_VAO;
  size_t                       m_verticesCount;
public:
  Mesh(const std::vector<Vertex> &verticices, const std::vector<unsigned int> &indicies);
  ~Mesh();

  void Draw();
};

}
