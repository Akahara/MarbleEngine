#pragma once

#include <vector>

#include <glad/glad.h>
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

// TODO do something with the VertexBufferLayout to avoid having to import glad with VertexBuffer
struct VertexBufferElement {

  unsigned int type;
  unsigned int count;
  unsigned char normalized;

  static constexpr unsigned int getSizeOfType(unsigned int type)
  {
	switch (type) {
	case GL_FLOAT:				return 4;
	case GL_UNSIGNED_INT:		return 4;
	case GL_UNSIGNED_BYTE:		return 1;
	default:                    return 0;
	}
  }

};

class VertexBufferLayout {
private:
  std::vector<VertexBufferElement> m_elements;
  unsigned int m_stride;

public:

  VertexBufferLayout() : m_stride(0) {}

  template<typename T>
  void push(unsigned int count);

  template<>
  void push<float>(unsigned int count)
  {
	m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
	m_stride += VertexBufferElement::getSizeOfType(GL_FLOAT) * count; // 4 bytes
  }

  template<>
  void push<glm::vec2>(unsigned int count)
  {
	m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
	m_stride += 2 * sizeof(GL_FLOAT) * count; // 4 bytes
  }

  template<>
  void push<glm::vec3>(unsigned int count)
  {
	m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
	m_stride += 3 * sizeof(GL_FLOAT) * count; // 4 bytes
  }

  template<>
  void push<glm::vec4>(unsigned int count)
  {
	m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
	m_stride += 4 * sizeof(GL_FLOAT) * count; // 4 bytes
  }

  template<>
  void push<unsigned int>(unsigned int count)
  {
	m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
	m_stride += VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT) * count;
  }

  template<>
  void push<unsigned char *>(unsigned int count)
  {
	m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
	m_stride += VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE) * count;
  }

  inline const std::vector<VertexBufferElement> &getElements() const { return m_elements; }
  inline unsigned int getStride() const { return m_stride; }
};

}

