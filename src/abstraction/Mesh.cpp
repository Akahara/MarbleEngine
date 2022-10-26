#include "Mesh.h"

#include "../Utils/BoundingSphere.h"

#include <iostream>

#include <glad/glad.h>

namespace Renderer {

static const VertexBufferLayout &getVertexBufferLayout()
{
  static VertexBufferLayout layout = []() {
    VertexBufferLayout l;
    l.push<float>(3); // position
    l.push<float>(2); // uv
    l.push<float>(3); // normal
    l.push<float>(1); // texID
    l.push<float>(3); // color
    return l;
  }();
  return layout;
}

Mesh::Mesh()
  : m_VBO(),
  m_IBO(),
  m_VAO(),
  m_verticesCount(0),
    m_vertices()
{
  
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
  : m_VBO(vertices.data(), sizeof(Vertex) * vertices.size()),
  m_IBO(indices.data(), indices.size()),
  m_VAO(),
  m_verticesCount((unsigned int)indices.size()),
    m_vertices(vertices)
{
  m_VAO.addBuffer(m_VBO, getVertexBufferLayout(), m_IBO);
  VertexArray::unbind();
  computeBoundingBox();

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

void Mesh::draw() const
{
  m_VAO.bind();
  glDrawElements(GL_TRIANGLES, m_verticesCount, GL_UNSIGNED_INT, nullptr);
  VertexArray::unbind();
}
void Mesh::moveBoundingBox(const glm::vec3& origin) const {
    m_boudingBox.setOrigin(origin);
}

void Mesh::computeBoundingBox() const {
    //Create a bounding sphere using the center of the vertices

    glm::vec3 avgPosition(0.F);
    const float inverse = 1.f / m_verticesCount;

    for (const auto& vertex : m_vertices) {
        avgPosition.x += vertex.position.x * inverse;
        avgPosition.y += vertex.position.y * inverse;
        avgPosition.z += vertex.position.z * inverse;
    }

    // Loop through every vertex and check the furthest away

    float maxDistance = 0.F;

    for (const auto& vertex : m_vertices) {
        maxDistance = glm::max(glm::distance(avgPosition, vertex.position), maxDistance);
    }

    // Create a bounding Sphere

    BoundingSphere bs = BoundingSphere(avgPosition, maxDistance);

    // Convert to AABB

    m_boudingBox = AABB::makeAABBfromBoundingSphere(bs);

    // Clear the vertices vector as we dont need it anymore and it eats memory
    //

    return;

}

}
