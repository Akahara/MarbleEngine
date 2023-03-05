#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace Renderer {

/* Immediate wrapper of the GL concept */
class VertexBufferObject {
private:
  unsigned int m_renderID;
public:
  VertexBufferObject() : m_renderID(0) {}
  VertexBufferObject(const void *vertices, size_t size); // can be constructed with null vertices
  VertexBufferObject(VertexBufferObject &&moved) noexcept;
  VertexBufferObject &operator=(VertexBufferObject &&moved) noexcept;
  ~VertexBufferObject();

  void bind() const;
  void unbind() const;

  // Unsafe
  unsigned int getId() const { return m_renderID; }
  // buffer must be bound
  void replaceData(const void *data, size_t size);
  void updateData(const void *data, size_t size, size_t offset=0);
};

struct VertexBufferElement {

  unsigned int type;
  unsigned int count;
  unsigned char normalized;

  static unsigned int getSizeOfType(unsigned int glType);

};

// TODO make constexpr
class VertexBufferLayout {
private:
  std::vector<VertexBufferElement> m_elements;
  unsigned int m_stride;

public:

  VertexBufferLayout() : m_stride(0) {}

  /* T may be (unsigned)int,float and glm vectors, matrices are not yet supported */
  template<typename T>
  VertexBufferLayout &push(unsigned int count);

  inline const std::vector<VertexBufferElement> &getElements() const { return m_elements; }
  inline unsigned int getStride() const { return m_stride; }
};

}

