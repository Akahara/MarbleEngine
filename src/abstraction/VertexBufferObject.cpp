#include "VertexBufferObject.h"

namespace Renderer {


    VertexBufferObject::VertexBufferObject(const void *vertices, size_t size) {
        glGenBuffers(1, &m_renderID);
        glBindBuffer(GL_ARRAY_BUFFER, m_renderID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    VertexBufferObject::VertexBufferObject(size_t size) {
        glGenBuffers(1, &m_renderID);
        glBindBuffer(GL_ARRAY_BUFFER, m_renderID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    VertexBufferObject::~VertexBufferObject() {
        destroy();
    }

    VertexBufferObject::VertexBufferObject(VertexBufferObject &&moved) noexcept
    {
        m_renderID = moved.m_renderID;
        moved.m_renderID = 0;
    }

    VertexBufferObject &VertexBufferObject::operator=(VertexBufferObject &&moved) noexcept
    {
        destroy();
        new (this)VertexBufferObject(std::move(moved));
        return *this;
    }

    void VertexBufferObject::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_renderID);
    }

    void VertexBufferObject::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBufferObject::destroy() {
        glDeleteBuffers(1, &m_renderID);
        m_renderID = 0;
    }
}