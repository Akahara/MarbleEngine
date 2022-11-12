#pragma once

#include "../../Shader.h"
#include "../../UnifiedRenderer.h"


class BloomRenderer
{
public:
    struct BloomMip {

        glm::vec2 resolution;
        Renderer::Texture texture;

        BloomMip(const glm::vec2& res) {

            resolution = res;
            texture = Renderer::Texture(res.x, res.y);

        }

    };
private:

    const int m_numberOfMips = 1;

    Renderer::Shader m_DownsampleShader = Renderer::loadShaderFromFiles("res/shaders/blit.vs", "res/shaders/bloom/downsampling.fs");
    Renderer::Shader m_UpsampleShader   = Renderer::loadShaderFromFiles("res/shaders/blit.vs", "res/shaders/bloom/upsampling.fs");


    std::vector<BloomMip*>          m_mipChain;
    Renderer::FrameBufferObject     m_fbo;

    Renderer::BlitPass              m_blitdata; //draw in the texture
    Renderer::BlitPass              m_blittest; //draw in the texture

public:
    BloomRenderer() 
    {  
        
        
        
        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++)

        {
            // Generate a mip with half the resolution of the previous mip
            float coef = std::pow(2, bloomPass+1);
            BloomMip* mip = new BloomMip{ glm::vec2(Window::getWinWidth() / coef, Window::getWinHeight() / coef) };
            m_mipChain.emplace_back(mip);
        }
        
    
    }

    void RenderBloomTexture(const Renderer::Texture& texture, float filterRadius) {

        m_blitdata.setShader("res/shaders/bloom/downsampling.fs");
        RenderDownsamples(texture);
        m_blitdata.setShader("res/shaders/bloom/upsampling.fs");
        //RenderUpsamples(filterRadius);

    }

private:

    void RenderDownsamples(const Renderer::Texture& texture) 
    {
        
        // Set the downsampling shader
        Renderer::Shader::unbind();
        m_blitdata.getShader().bind();
        
        texture.bind();
        m_blitdata.getShader().setUniform2f("u_srcResolution", glm::vec2(Window::getWinWidth(), Window::getWinHeight())    );
        //m_blitdata.getShader().setUniform1i("u_texture", texture.getId());

        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++) 
        
        {
            BloomMip& mip = *m_mipChain[bloomPass];

            // Set fbo target to new mips texture 

            Renderer::FrameBufferObject::setViewportToTexture(mip.texture);
            m_fbo.setTargetTexture(mip.texture);


            // Get the last mip textur

            m_fbo.bind();
            m_blitdata.doBlit(texture);
            m_fbo.unbind();
            

            m_blittest.doBlit(mip.texture);
            
            m_blitdata.getShader().bind();
            m_blitdata.getShader().setUniform2f("u_srcResolution", mip.resolution);
            m_blitdata.getShader().unbind();
            /*
            else {


                m_fbo.bind();
                m_blitdata.doBlit(m_mipChain.at(bloomPass-1)->texture); // get the previous mip texture
                m_fbo.unbind();

            }
            */

        }
        
    }


    void RenderUpsamples(float filterRadius) 
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);


        // ...

        glDisable(GL_BLEND);

    }


};