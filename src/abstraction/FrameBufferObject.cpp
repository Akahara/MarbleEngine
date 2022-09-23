#include "FrameBufferObject.h"

#include "Renderer.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 800

namespace Renderer {


    FrameBufferObject::FrameBufferObject()
        
    {
        GLCall(glGenFramebuffers(1, &m_RenderID));
        GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RenderID));

        /* Texture */
        GLCall(glGenTextures(1, &m_TextureColorBuffer));
        GLCall(glBindTexture(GL_TEXTURE_2D, m_TextureColorBuffer));

        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));

        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureColorBuffer, 0));


        /* Renderbuffer */
        
        GLCall(glGenRenderbuffers(1, &m_RenderBuffer));
        GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer));
        GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT));
        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer));
                          

        /* Binding */

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Framebuffer ok, id : " << m_RenderID << std::endl;
            std::cout << "textureColorbuffer id : " << m_TextureColorBuffer << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);





    }

    FrameBufferObject::~FrameBufferObject() {
        Delete();
    }

    void FrameBufferObject::Bind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RenderID));
        GLCall(glEnable(GL_DEPTH_TEST));
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void FrameBufferObject::Unbind() const {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        GLCall(glDisable(GL_DEPTH_TEST));
        GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

    }

    void FrameBufferObject::Delete() {

        GLCall(glDeleteFramebuffers(1, &m_RenderID));

    }
}