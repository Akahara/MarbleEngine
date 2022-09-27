#include "VertexBufferObject.h"
#include "Renderer.h"


namespace Renderer {


    VertexBufferObject::VertexBufferObject(const void *vertices, size_t size) {
        glGenBuffers(1, &m_RenderID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RenderID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    VertexBufferObject::VertexBufferObject(size_t size) {
        glGenBuffers(1, &m_RenderID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RenderID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    VertexBufferObject::~VertexBufferObject() {
        Delete();
    }

    VertexBufferObject::VertexBufferObject(VertexBufferObject &&moved) noexcept
    {
        m_RenderID = moved.m_RenderID;
        moved.m_RenderID = 0;
    }

    VertexBufferObject &VertexBufferObject::operator=(VertexBufferObject &&moved) noexcept
    {
        Delete();
        new (this)VertexBufferObject(std::move(moved));
        return *this;
    }

    void VertexBufferObject::Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_RenderID);
    }

    void VertexBufferObject::Unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBufferObject::Delete() {
        glDeleteBuffers(1, &m_RenderID);
        m_RenderID = 0;
    }
}