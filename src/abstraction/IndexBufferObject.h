#pragma once

#include "BufferObject.h"
#define GLFW_INCLUDE_NONE
#include<glad/glad.h>

namespace Renderer {

    class IndexBufferObject : public BufferObject
    {
    private:
        unsigned int m_Count;
    public :

        IndexBufferObject() : m_Count(0) {}
        IndexBufferObject(unsigned int* indices, unsigned int count);
        ~IndexBufferObject();

        void Bind() const override;
        void Unbind() const override;
        void Delete() override ;

        const unsigned int getCount() const { return m_Count; }

    };
}

