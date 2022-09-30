#include "Mesh.h"

#include <glad/glad.h>

namespace Renderer {

static const VertexBufferLayout &getVertexBufferLayout()
{
  static VertexBufferLayout layout = []() {
    VertexBufferLayout l;
    l.push<float>(3); // position
    l.push<float>(2); // uv
    l.push<float>(3); // normal
    return l;
  }();
  return layout;
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
  : m_VBO(sizeof(Vertex) * vertices.size()),
  m_IBO(indices.data(), indices.size()),
  m_VAO(),
  m_verticesCount(indices.size())
{
  m_VAO.addBuffer(m_VBO, getVertexBufferLayout(), m_IBO);
  m_VAO.Unbind();
}

Mesh::~Mesh()
{
}

void Mesh::Draw()
{
  m_VAO.Bind();
  glDrawElements(GL_TRIANGLES, m_verticesCount * 3, GL_UNSIGNED_INT, nullptr);
  m_VAO.Unbind();
}

}
