#include "Mesh.h"

#include <glad/glad.h>
#include <iostream>

namespace Renderer {

static const VertexBufferLayout &getVertexBufferLayout()
{
  static VertexBufferLayout layout = []() {
    VertexBufferLayout l;
    l.push<float>(3); // position
    l.push<float>(2); // uv
    l.push<float>(3); // normal
    l.push<float>(1); // texID
    return l;
  }();
  return layout;
}

Mesh::Mesh()
  : m_VBO(),
  m_IBO(),
  m_VAO(),
  m_verticesCount(0)
{
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
  : m_VBO(vertices.data(), sizeof(Vertex) * vertices.size()),
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

Mesh::Mesh(Mesh &&moved) noexcept
  : m_VAO(std::move(moved.m_VAO)),
  m_VBO(std::move(moved.m_VBO)),
  m_IBO(std::move(moved.m_IBO)),
  m_verticesCount(moved.m_verticesCount)
{
  moved.m_verticesCount = 0;
}

Mesh &Mesh::operator=(Mesh &&moved) noexcept
{
  this->~Mesh();
  new (this)Mesh(std::move(moved));
  return *this;
}

void Mesh::Draw() const
{
  m_VAO.Bind();
  glDrawElements(GL_TRIANGLES, m_verticesCount, GL_UNSIGNED_INT, nullptr);
  m_VAO.Unbind();
}

}
