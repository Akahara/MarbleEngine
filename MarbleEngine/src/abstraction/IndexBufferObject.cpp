#include "IndexBufferObject.h"

#include <memory>

#include <glad/glad.h>

namespace Renderer {

IndexBufferObject::IndexBufferObject(const unsigned int* indices, size_t count)
  : m_count(count)
{
  glGenBuffers(1, &m_renderID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

IndexBufferObject::~IndexBufferObject() {
  destroy();
}

IndexBufferObject::IndexBufferObject(IndexBufferObject &&moved) noexcept
{
  m_count = moved.m_count;
  m_renderID = moved.m_renderID;
  moved.m_renderID = 0;
  moved.m_count = 0;
}

IndexBufferObject& IndexBufferObject::operator=(IndexBufferObject &&moved) noexcept
{
  destroy();
  new (this) IndexBufferObject(std::move(moved));
  return *this;
}

void IndexBufferObject::bind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderID);
}

void IndexBufferObject::unbind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBufferObject::destroy() {
  glDeleteBuffers(1, &m_renderID);
  m_renderID = 0;
}

}