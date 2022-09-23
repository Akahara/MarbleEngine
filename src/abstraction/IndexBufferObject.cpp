#include "IndexBufferObject.h"
#include "Renderer.h"


namespace Renderer {


    IndexBufferObject::IndexBufferObject(unsigned int* indices, unsigned int count)
        : m_Count(count)
    {

        GLCall(glGenBuffers(1, &m_RenderID));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID));

        GLCall(;);
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    }

    IndexBufferObject::~IndexBufferObject() {
        Delete();
    }

    void IndexBufferObject::Bind() const {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RenderID));
    }

    void IndexBufferObject::Unbind() const {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    void IndexBufferObject::Delete() {

        GLCall(glDeleteBuffers(1, &m_RenderID));

    }
}