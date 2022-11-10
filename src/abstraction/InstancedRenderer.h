#pragma once

#include "VertexBufferObject.h"
#include "VertexArray.h"

namespace Renderer {

struct IndirectDrawCommand {
  GLuint count;
  GLuint instanceCount;
  GLuint firstIndex;
  GLint  baseVertex;
  GLuint baseInstance;
};

template<class Vertex_t, size_t N>
class VertexBufferHolder {
public:
  using Vertex = Vertex_t;
private:
  std::array<VertexBufferObject, N> m_vbos;
  std::array<IndexBufferObject, N> m_ibos;
  VertexBufferLayout m_layout;
  size_t m_bufferCount;
public:
  VertexBufferHolder()
  {
    m_layout = Vertex_t::getLayout();
  }

  VertexBufferHolder(const VertexBufferHolder &) = delete;
  VertexBufferHolder& operator=(const VertexBufferHolder &) = delete;
  VertexBufferHolder(VertexBufferHolder &&moved) noexcept
  {
    m_vbos = std::move(moved.m_vbos);
    m_ibos = std::move(moved.m_ibos);
    m_layout = moved.m_layout;
    m_bufferCount = moved.m_bufferCount;
  }
  VertexBufferHolder &operator=(VertexBufferHolder &&moved) noexcept
  {
    this->~VertexBufferHolder();
    new (this)VertexBufferHolder(std::move(moved));
    return *this;
  }

  size_t buildBuffer(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
  {
    m_vbos[m_bufferCount] = VertexBufferObject(vertices.data(), vertices.size() * sizeof(Vertex));
    m_ibos[m_bufferCount] = IndexBufferObject(indices.data(), indices.size());
    return m_bufferCount++;
  }

  void emplaceBuffer(size_t id, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
  {
    m_vbos[id] = VertexBufferObject(vertices.data(), vertices.size() * sizeof(Vertex));
    m_ibos[id] = IndexBufferObject(indices.data(), indices.size());
  }

  VertexBufferObject &getBuffer(size_t id) { return m_vbos[id]; }
  IndexBufferObject &getIndexBuffer(size_t id) { return m_ibos[id]; }
  VertexBufferLayout &getLayout() { return m_layout; }

  void bindBuffer(size_t id, VertexArray &vao) const
  {
    vao.addBuffer(m_vbos[id], m_layout, m_ibos[id]);
  }
};

//template<class ModelVertex_t, class InstanceData_t, size_t LOD=1>
//class InstancedMesh {
//  static_assert(sizeof(InstanceData_t) % 4 == 0); // beware of padding! TODO comment better
//public:
//  using ModelVertex = ModelVertex_t;
//  using InstanceData = InstanceData_t;
//
//  VertexBufferHolder<ModelVertex_t, LOD> m_models;
//  VertexBufferObject m_instanceVBO;
//  VertexArray m_vao;
//  unsigned int m_instanceCount;
//
//public:
//  InstancedMesh()
//    : m_models(), m_instanceVBO(), m_ibo(), m_vao(), m_instanceCount(0)
//  {
//  }
//
//  InstancedMesh(size_t instanceCount)
//    : m_models(),
//    m_instanceVBO(instanceCount * sizeof(InstanceData)),
//    m_instanceCount(instanceCount),
//    m_vao()
//  {
//    VertexBufferLayout modelLayout = ModelVertex::getLayout();
//    VertexBufferLayout instanceLayout = InstanceData::getLayout();
//    m_vao.addBuffer(m_modelVBO, modelLayout, m_ibo);
//    m_vao.addInstanceBuffer(m_instanceVBO, instanceLayout, modelLayout);
//    m_vao.unbind();
//  }
//
//  InstancedMesh(InstancedMesh &&moved) noexcept
//  {
//    m_modelVBO = std::move(moved.m_modelVBO);
//    m_instanceVBO = std::move(moved.m_instanceVBO);
//    m_ibo = std::move(moved.m_ibo);
//    m_vao = std::move(moved.m_vao);
//    m_instanceCount = moved.m_instanceCount;
//  }
//
//  InstancedMesh &operator=(InstancedMesh &&moved) noexcept
//  {
//    this->~InstancedMesh();
//    new (this)InstancedMesh(std::move(moved));
//    return *this;
//  }
//
//  InstancedMesh(const InstancedMesh &) = delete;
//  InstancedMesh &operator=(InstancedMesh &moved) = delete;
//
//  VertexBufferHolder getModels() { return m_models; }
//
//  void setVBO(const VertexBufferObject &vbo, const IndexBufferObject &ibo)
//  {
//    m_vao.addBuffer(vbo, ModelVertex::getLayout(), ibo);
//  }
//
//  void draw() const
//  {
//    m_vao.bind();
//    glDrawElementsInstanced(GL_TRIANGLES, (unsigned int)m_ibo.getCount(), GL_UNSIGNED_INT, nullptr, m_instanceCount);
//    m_vao.unbind();
//  }
//};

}