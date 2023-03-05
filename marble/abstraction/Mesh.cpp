#include "Mesh.h"

#include <iostream>

#include <map>

#include <glad/glad.h>

#include "../Utils/BoundingSphere.h"
#include "../Utils/Mathf.h"

namespace Renderer {

Model::Model()
  : m_VBO(),
    m_IBO(),
    m_verticesCount(0),
    m_boudingBox()
{
}

Model::Model(const std::vector<BaseVertex> &vertices, const std::vector<unsigned int> &indices)
  : m_verticesCount((unsigned int)indices.size())
{
  m_VBO = VertexBufferObject(vertices.data(), sizeof(BaseVertex) * vertices.size());
  m_IBO = IndexBufferObject(indices.data(), indices.size());
  VertexArray::unbind();

  glm::vec3 aabbMin{ std::numeric_limits<float>::max() };
  glm::vec3 aabbMax{ std::numeric_limits<float>::min() };
  for (const BaseVertex &v : vertices) {
    aabbMin = glm::min(aabbMin, v.position);
    aabbMax = glm::max(aabbMax, v.position);
  }
  m_boudingBox = AABB::make_aabb(aabbMin, aabbMax);
}

Model::Model(Model &&moved) noexcept
  : m_VBO(std::move(moved.m_VBO)),
    m_IBO(std::move(moved.m_IBO)),
    m_verticesCount(moved.m_verticesCount),
    m_boudingBox(moved.m_boudingBox)
{
  moved.m_verticesCount = 0;
}

Model &Model::operator=(Model &&moved) noexcept
{
  this->~Model();
  new (this)Model(std::move(moved));
  return *this;
}

Mesh::Mesh(const std::shared_ptr<Model> &model, const std::shared_ptr<Material> &material)
  : m_model(model), m_material(material), m_transform(), m_VAO()
{
  m_VAO.addBuffer(model->getVBO(), BaseVertex::getVertexBufferLayout(), model->getIBO());
  m_VAO.unbind();
}

Mesh::Mesh(Mesh &&moved) noexcept
  : m_model(std::move(moved.m_model)),
    m_material(std::move(moved.m_material)),
    m_transform(moved.m_transform),
    m_VAO(std::move(moved.m_VAO))
{
}

Mesh &Mesh::operator=(Mesh &&moved) noexcept
{
  this->~Mesh();
  new (this)Mesh(std::move(moved));
  return *this;
}

AABB Model::getBoundingBoxInstance(glm::vec3 instancePosition, glm::vec3 instanceSize) const
{
  return AABB(instancePosition - (m_boudingBox.getSize() * instanceSize)/2.f, m_boudingBox.getSize() * instanceSize);
}

InstancedMesh::InstancedMesh(const std::shared_ptr<Model> &model, const std::shared_ptr<Material> &material, size_t instanceSize, size_t instanceCount)
  : m_model(model),
    m_material(material),
    m_instanceSize(instanceSize),
    m_instanceCount(instanceCount),
    m_instanceBuffer(nullptr, instanceSize*instanceCount)
{
  m_VAO.addBuffer(model->getVBO(), BaseVertex::getVertexBufferLayout(), model->getIBO());
  m_VAO.addInstanceBuffer(m_instanceBuffer, BaseInstance::getVertexBufferLayout(), BaseVertex::getVertexBufferLayout());
  m_VAO.unbind();
}

InstancedMesh::InstancedMesh(InstancedMesh &&moved) noexcept
  : m_model(std::move(moved.m_model)),
    m_material(std::move(moved.m_material)),
    m_VAO(std::move(moved.m_VAO)),
    m_instanceBuffer(std::move(moved.m_instanceBuffer)),
    m_instanceCount(moved.m_instanceCount),
    m_instanceSize(moved.m_instanceSize)
{
}

InstancedMesh &InstancedMesh::operator=(InstancedMesh &&moved) noexcept
{
  this->~InstancedMesh();
  new (this)InstancedMesh(std::move(moved));
  return *this;
}

void InstancedMesh::updateInstances(const BaseInstance *data, size_t beginInstance, size_t endInstance)
{
  assert(endInstance <= m_instanceCount);
  assert(beginInstance <= endInstance);
  m_instanceBuffer.bind();
  m_instanceBuffer.updateData(data, (endInstance - beginInstance) * m_instanceSize, beginInstance * m_instanceSize);
  m_instanceBuffer.unbind();
}


NormalsMesh::NormalsMesh()
  : m_VBO(),
  m_IBO(),
  m_VAO(),
  m_verticesCount(0)
{
}

NormalsMesh::NormalsMesh(const std::vector<BaseVertex> &vertices)
{
  std::vector<BaseVertex> newVertices;
  newVertices.resize(vertices.size() * 2);
  std::vector<unsigned int> newIndices;
  newIndices.resize(vertices.size() * 2);

  for (size_t i = 0; i < vertices.size(); i++) {
    const BaseVertex &v = vertices[i];
    newVertices[2*i]   = v;
    newVertices[2*i+1] = v;
    newVertices[2*i+1].position = v.position + v.normal;
  }
  for (size_t i = 0; i < newVertices.size(); i++)
    newIndices[i] = (unsigned int)i;

  m_VBO = VertexBufferObject(newVertices.data(), sizeof(BaseVertex) * newVertices.size());
  m_IBO = IndexBufferObject(newIndices.data(), newIndices.size());
  m_VAO.addBuffer(m_VBO, BaseVertex::getVertexBufferLayout(), m_IBO);
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
