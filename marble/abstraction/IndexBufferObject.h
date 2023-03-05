#pragma once

#include <stddef.h>
#include <initializer_list>

namespace Renderer {

/* Immediate wrapper of the GL concept */
class IndexBufferObject {
private:
  unsigned int m_renderID;
  size_t m_count;
public:
  IndexBufferObject() : m_renderID(0), m_count(0) {}
  IndexBufferObject(const unsigned int *indices, size_t count);
  IndexBufferObject(const std::initializer_list<unsigned int> &indices) : IndexBufferObject(indices.begin(), indices.size()) {}
  ~IndexBufferObject() noexcept;
  IndexBufferObject(IndexBufferObject &&moved) noexcept;
  IndexBufferObject &operator=(IndexBufferObject &&moved) noexcept;

  void bind() const;
  void unbind() const;

  size_t getCount() const { return m_count; }
};

}

