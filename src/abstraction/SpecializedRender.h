#pragma once

#include "VertexBufferObject.h"
#include "VertexArray.h"

/**
* Specialized rendering functions, used by the grass renderer.
* 
* Contains
* - VertexBufferHolder : holds multiple IBO/VBO, used with LD/HD rendering
* - IndirectDrawCommand : the GL object that defines glDrawElementsInstancedIndirect uses (to be instanced on the gpu, not in cpu memory)
*/
namespace Renderer {

struct IndirectDrawCommand {
  /*GLuint*/unsigned int count;
  /*GLuint*/unsigned int instanceCount;
  /*GLuint*/unsigned int firstIndex;
  /*GLint*/ int          baseVertex;
  /*GLuint*/unsigned int baseInstance;
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

}