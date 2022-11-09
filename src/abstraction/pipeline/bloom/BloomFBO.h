#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../../FrameBufferObject.h"
#include "../../Texture.h"
#include "../../Window.h"

#include <glad/glad.h>

struct BloomMip {

	glm::vec2 size;
	glm::ivec2 intSize;
	Renderer::Texture texture;
};


class BloomFBO {


private:
    bool m_isInit;
    std::vector<BloomMip*> m_mipChain; // switch to ptr


public:
    Renderer::FrameBufferObject m_FBO;
    BloomFBO() : m_isInit(false) {}

    void init(unsigned int mipChainLength) {

        if (m_isInit) return;

        glm::vec2 mipSize(Window::getWinWidth(), Window::getWinHeight());
        glm::ivec2 mipIntSize(Window::getWinWidth(), Window::getWinHeight());
        for (unsigned int i = 0; i < mipChainLength; i++)
        {

            mipSize *= 0.5f;
            mipIntSize /= 2;


            BloomMip *mip = new BloomMip{
                mipSize,
                mipIntSize,
                Renderer::Texture(mipSize.x, mipSize.y)
            };
            m_mipChain.emplace_back(mip); 
        }
        m_FBO.setTargetTexture(m_mipChain[0]->texture);

        // wtf does this do
        unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachments);

        m_FBO.unbind();
        m_isInit = true;
        /*

        */
    }

    void bindForWriting() { m_FBO.bind(); }
    
    std::vector<BloomMip*>& getMipChain() { return m_mipChain; }

};