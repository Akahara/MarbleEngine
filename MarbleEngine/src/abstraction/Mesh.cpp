#include "Mesh.h"

#include <iostream>

#include <map>

#include <glad/glad.h>

#include "../Utils/BoundingSphere.h"
#include "../Utils/Mathf.h"

namespace Renderer {

static const VertexBufferLayout &getVertexBufferLayout()
{
  static VertexBufferLayout layout = []() {
    VertexBufferLayout l;
    l.push<float>(3); // position
    l.push<float>(2); // uv
    l.push<float>(3); // normal
    l.push<float>(3); // color
    l.push<float>(1); // texId
    return l;
  }();
  return layout;
}

Mesh::Mesh()
  : m_VBO(),
  m_IBO(),
  m_VAO(),
  m_verticesCount(0),
  m_boudingBox()
{
}

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices,
    const std::unordered_map<int, std::shared_ptr<Texture>>& slotsTextures)
  : m_VBO(vertices.data(), sizeof(Vertex) * vertices.size()),
  m_IBO(indices.data(), indices.size()),
  m_VAO(),
  m_verticesCount((unsigned int)indices.size()),
  m_SlotTextures(slotsTextures)
{
  m_VAO.addBuffer(m_VBO, getVertexBufferLayout(), m_IBO);
  VertexArray::unbind();
  
  glm::vec3 aabbMin{ std::numeric_limits<float>::max() };
  glm::vec3 aabbMax{ std::numeric_limits<float>::min() };
  for (const Vertex &v : vertices) {
    aabbMin = glm::min(aabbMin, v.position);
    aabbMax = glm::max(aabbMax, v.position);
  }
  m_boudingBox = AABB::make_aabb(aabbMin, aabbMax);
  
}

Mesh::~Mesh()
{
}

Mesh::Mesh(Mesh &&moved) noexcept
  : m_VAO(std::move(moved.m_VAO)),
  m_VBO(std::move(moved.m_VBO)),
  m_IBO(std::move(moved.m_IBO)),
  m_verticesCount(moved.m_verticesCount),
  m_boudingBox(moved.m_boudingBox)
{
  moved.m_verticesCount = 0;
}

Mesh &Mesh::operator=(Mesh &&moved) noexcept
{
  this->~Mesh();
  new (this)Mesh(std::move(moved));
  return *this;
}

void Mesh::bindTextureToSlot(const std::shared_ptr<Texture>& texture, int slot) {


    m_SlotTextures[slot] = texture;
}


void Mesh::draw() const
{
    
  m_VAO.bind();

  // bind all textures
  for (const auto& [slot, texture_ptr] : m_SlotTextures) {
      texture_ptr->bind(slot);
  }
  
  glDrawElements(GL_TRIANGLES, m_verticesCount, GL_UNSIGNED_INT, 0);

  VertexArray::unbind();
}

AABB Mesh::getBoundingBoxInstance(glm::vec3 instancePosition, glm::vec3 instanceSize) const
{

  return AABB(instancePosition - (m_boudingBox.getSize() * instanceSize)/2.f, m_boudingBox.getSize() * instanceSize);
  //return AABB(m_boudingBox.getOrigin() + instancePosition, m_boudingBox.getSize() * instanceSize);
}

NormalsMesh::NormalsMesh()
  : m_VBO(),
  m_IBO(),
  m_VAO(),
  m_verticesCount(0)
{
}

NormalsMesh::NormalsMesh(const std::vector<Vertex> &vertices)
{
  std::vector<Vertex> newVertices;
  newVertices.resize(vertices.size() * 2);
  std::vector<unsigned int> newIndices;
  newIndices.resize(vertices.size() * 2);

  for (size_t i = 0; i < vertices.size(); i++) {
    const Vertex &v = vertices[i];
    newVertices[2*i]   = v;
    newVertices[2*i+1] = v;
    newVertices[2*i+1].position = v.position + v.normal;
  }
  for (size_t i = 0; i < newVertices.size(); i++)
    newIndices[i] = (unsigned int)i;

  m_VBO = VertexBufferObject(newVertices.data(), sizeof(Vertex) * newVertices.size());
  m_IBO = IndexBufferObject(newIndices.data(), newIndices.size());
  m_VAO.addBuffer(m_VBO, getVertexBufferLayout(), m_IBO);
  m_verticesCount = (unsigned int)newVertices.size();
  VertexArray::unbind();
}

NormalsMesh::~NormalsMesh()
{
}

NormalsMesh::NormalsMesh(NormalsMesh &&moved) noexcept
  : m_VAO(std::move(moved.m_VAO)),
  m_VBO(std::move(moved.m_VBO)),
  m_IBO(std::move(moved.m_IBO)),
  m_verticesCount(moved.m_verticesCount)
{
  moved.m_verticesCount = 0;
}

NormalsMesh &NormalsMesh::operator=(NormalsMesh &&moved) noexcept
{
  this->~NormalsMesh();
  new (this)NormalsMesh(std::move(moved));
  return *this;
}

void NormalsMesh::draw() const
{
  m_VAO.bind();
  glDrawElements(GL_LINES, m_verticesCount, GL_UNSIGNED_INT, nullptr);
  VertexArray::unbind();
}

}
