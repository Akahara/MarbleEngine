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
  VertexBufferObject m_VBO;
  IndexBufferObject  m_IBO;
  VertexArray        m_VAO;
  unsigned int       m_verticesCount;
public:
  Mesh();
  Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indicies);
  ~Mesh();
  Mesh(Mesh &&moved) noexcept;
  Mesh &operator=(Mesh &&moved) noexcept;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(const Mesh &) = delete;

  void Draw() const;
};

}
