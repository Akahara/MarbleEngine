#include "IndexBufferObject.h"

#include <memory>

#include <glad/glad.h>

namespace Renderer {

IndexBufferObject::IndexBufferObject(const unsigned int* indices, size_t count)
  : m_Count(count)
{
  glGenBuffers(1, &m_RenderID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

IndexBufferObject::~IndexBufferObject() {
  Delete();
}

IndexBufferObject::IndexBufferObject(IndexBufferObject &&moved) noexcept
{
  m_Count = moved.m_Count;
  m_RenderID = moved.m_RenderID;
  moved.m_RenderID = 0;
  moved.m_Count = 0;
}

IndexBufferObject& IndexBufferObject::operator=(IndexBufferObject &&moved) noexcept
{
  Delete();
  new (this) IndexBufferObject(std::move(moved));
  return *this;
}

void IndexBufferObject::Bind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID);
}

void IndexBufferObject::Unbind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBufferObject::Delete() {
  glDeleteBuffers(1, &m_RenderID);
  m_RenderID = 0;
}

}