#pragma once

#include "BufferObject.h"

namespace Renderer {

    class VertexBufferObject : public BufferObject
    {
    public :
        VertexBufferObject() {}
        VertexBufferObject(const void* vertices, unsigned int size);
        VertexBufferObject(unsigned int size);
        ~VertexBufferObject();
        VertexBufferObject(VertexBufferObject &&moved) noexcept;
        VertexBufferObject &operator=(VertexBufferObject &&moved) noexcept;

        void Bind() const override;
        void Unbind() const override;
        void Delete() override;

    };
}

