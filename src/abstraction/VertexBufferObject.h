#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "BufferObject.h"

namespace Renderer {

class VertexBufferObject : public BufferObject {
public:
  VertexBufferObject() {}
  VertexBufferObject(const void *vertices, size_t size);
  VertexBufferObject(size_t size);
  ~VertexBufferObject();
  VertexBufferObject(VertexBufferObject &&moved) noexcept;
  VertexBufferObject &operator=(VertexBufferObject &&moved) noexcept;

  void bind() const override;
  void unbind() const override;
  void destroy() override;

};

struct VertexBufferElement {

  unsigned int type;
  unsigned int count;
  unsigned char normalized;

  static unsigned int getSizeOfType(unsigned int glType);

};

class VertexBufferLayout {
private:
  std::vector<VertexBufferElement> m_elements;
  unsigned int m_stride;

public:

  VertexBufferLayout() : m_stride(0) {}

  /* T may be (unsigned)int,float and glm vectors */
  template<typename T>
  void push(unsigned int count);

  inline const std::vector<VertexBufferElement> &getElements() const { return m_elements; }
  inline unsigned int getStride() const { return m_stride; }
};

}

