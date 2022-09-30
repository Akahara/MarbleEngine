#pragma once

#include "BufferObject.h"

namespace Renderer {

    class IndexBufferObject : public BufferObject
    {
    private:
        size_t m_Count;
    public:
        IndexBufferObject() : m_Count(0) {}
        IndexBufferObject(const unsigned int* indices, size_t count);
        ~IndexBufferObject() noexcept;
        IndexBufferObject(IndexBufferObject &&moved) noexcept;
        IndexBufferObject& operator=(IndexBufferObject &&moved) noexcept;

        void Bind() const override;
        void Unbind() const override;
        void Delete() override;

        size_t getCount() const { return m_Count; }

    };
}

