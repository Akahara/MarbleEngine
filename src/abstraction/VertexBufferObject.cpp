#include "VertexBufferObject.h"
#include "Renderer.h"


namespace Renderer {


    VertexBufferObject::VertexBufferObject(const void *vertices, unsigned int size) {

        GLCall(glGenBuffers(1, &m_RenderID));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RenderID));
        GLCall(glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW));

    }

    VertexBufferObject::VertexBufferObject(unsigned int size) {

        GLCall(glGenBuffers(1, &m_RenderID));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RenderID));
        GLCall(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));

    }

    VertexBufferObject::~VertexBufferObject() {
        Delete();
    }

    void VertexBufferObject::Bind() const {
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RenderID));
    }

    void VertexBufferObject::Unbind() const {
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    void VertexBufferObject::Delete() {

        GLCall(glDeleteBuffers(1, &m_RenderID));

    }
}