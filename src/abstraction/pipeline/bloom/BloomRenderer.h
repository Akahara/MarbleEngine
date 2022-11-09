#pragma once

#include "../../Shader.h"
#include "../../UnifiedRenderer.h"


class BloomRenderer
{
private:

    const int m_numberOfMips = 5;

    Renderer::Shader m_DownsampleShader = Renderer::loadShaderFromFiles("res/shaders/blit.vs", "res/shaders/bloom/downsampling.fs");
    Renderer::Shader m_UpsampleShader   = Renderer::loadShaderFromFiles("res/shaders/blit.vs", "res/shaders/bloom/upsampling.fs");


    struct BloomMip {

        glm::vec2 resolution;
        Renderer::Texture texture;
    };

    std::vector<BloomMip*>          m_mipChain;
    Renderer::FrameBufferObject     m_fbo;

    Renderer::BlitPass              m_blitdata; //draw in the texture

public:
    BloomRenderer() 
    {  
    
    
    }

    void Destroy();
    void RenderBloomTexture(const Renderer::Texture& texture, float filterRadius) {

        Renderer::FrameBufferObject::setViewportToWindow();
        m_fbo.bind();
        RenderDownsamples(texture);
        RenderUpsamples(filterRadius);
        m_fbo.unbind();
    }

private:

    void RenderDownsamples(const Renderer::Texture& texture) 
    {
        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++) 
        
        {
            // Generate a mip with lower resolution

            // Set fbo target to new mips texture 

            // blit 

            




        }



       
    }


    void RenderUpsamples(float filterRadius) 
    {

    }


};