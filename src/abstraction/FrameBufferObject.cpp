#include "FrameBufferObject.h"

#include "Renderer.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 800

namespace Renderer {


    FrameBufferObject::FrameBufferObject()
    {
        glGenFramebuffers(1, &m_RenderID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RenderID);

        /* Texture */
        glGenTextures(1, &m_TextureColorBuffer);
        glBindTexture(GL_TEXTURE_2D, m_TextureColorBuffer);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureColorBuffer, 0);


        /* Renderbuffer */
        
        glGenRenderbuffers(1, &m_RenderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBuffer);
                          

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
        glBindFramebuffer(GL_FRAMEBUFFER, m_RenderID);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void FrameBufferObject::Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

    }

    void FrameBufferObject::Delete() {

        glDeleteFramebuffers(1, &m_RenderID);

    }
}