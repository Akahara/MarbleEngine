#include "VertexBufferObject.h"

#include <stdexcept>

#include <glad/glad.h>

namespace Renderer {

VertexBufferObject::VertexBufferObject(const void *vertices, size_t size)
{
    glGenBuffers(1, &m_renderID);
    glBindBuffer(GL_ARRAY_BUFFER, m_renderID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

VertexBufferObject::~VertexBufferObject()
{
    glDeleteBuffers(1, &m_renderID);
    m_renderID = 0;
}

VertexBufferObject::VertexBufferObject(VertexBufferObject &&moved) noexcept
{
    m_renderID = moved.m_renderID;
    moved.m_renderID = 0;
}

VertexBufferObject &VertexBufferObject::operator=(VertexBufferObject &&moved) noexcept
{
    this->~VertexBufferObject();
    new (this)VertexBufferObject(std::move(moved));
    return *this;
}

void VertexBufferObject::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_renderID);
}

void VertexBufferObject::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBufferObject::replaceData(const void *data, size_t size)
{
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void VertexBufferObject::updateData(const void *data, size_t size, size_t offset)
{
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

unsigned int VertexBufferElement::getSizeOfType(unsigned int glType)
{
    switch (glType) {
    case GL_FLOAT:         return 4;
    case GL_UNSIGNED_INT:  return 4;
    case GL_UNSIGNED_BYTE: return 1;
    default:               throw std::runtime_error("Unreachable");
    }
}

template<>
VertexBufferLayout &VertexBufferLayout::push<float>(unsigned int count)
{
    m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
    m_stride += VertexBufferElement::getSizeOfType(GL_FLOAT) * count; // 4 bytes
    return *this;
}

template<>
VertexBufferLayout &VertexBufferLayout::push<glm::vec2>(unsigned int count)
{
    return push<float>(2);
}

template<>
VertexBufferLayout &VertexBufferLayout::push<glm::vec3>(unsigned int count)
{
    return push<float>(3);
}

template<>
VertexBufferLayout &VertexBufferLayout::push<glm::vec4>(unsigned int count)
{
    return push<float>(4);
}

template<>
VertexBufferLayout &VertexBufferLayout::push<unsigned int>(unsigned int count)
{
    m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
    m_stride += VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT) * count;
    return *this;
}

}