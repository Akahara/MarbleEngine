#pragma once

#include <initializer_list>

#include "BufferObject.h"

namespace Renderer {

/* Immediate wrapper of the GL concept */
class IndexBufferObject : public BufferObject {
private:
  size_t m_count;
public:
  IndexBufferObject() : m_count(0) {}
  IndexBufferObject(const unsigned int *indices, size_t count);
  IndexBufferObject(const std::initializer_list<unsigned int> &indices) : IndexBufferObject(indices.begin(), indices.size()) {}
  ~IndexBufferObject() noexcept;
  IndexBufferObject(IndexBufferObject &&moved) noexcept;
  IndexBufferObject &operator=(IndexBufferObject &&moved) noexcept;

  void bind() const override;
  void unbind() const override;
  void destroy() override;

  size_t getCount() const { return m_count; }
};

}

