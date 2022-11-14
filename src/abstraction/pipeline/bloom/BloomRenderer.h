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
            texture = Renderer::Texture((unsigned int)res.x, (unsigned int)res.y);

        }

    };
private:

    const unsigned int m_numberOfMips = 5;

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
            double coef = std::pow(2, bloomPass+1);
            BloomMip* mip = new BloomMip{ glm::vec2(Window::getWinWidth() / coef, Window::getWinHeight() / coef) };
            m_mipChain.emplace_back(mip);
        }
        
    
    }

    void RenderBloomTexture(const Renderer::Texture& texture, float filterRadius, bool write = false) {

        m_fbo.bind();
        m_blitdata.setShader("res/shaders/bloom/downsampling.fs");
        RenderDownsamples(texture, write);
        m_blitdata.setShader("res/shaders/bloom/upsampling.fs");
        RenderUpsamples(filterRadius, write);
        m_fbo.unbind();

             

    }

    Renderer::Texture* getFinalBloomTexture() const { return &m_mipChain[0]->texture; }

private:

    void RenderDownsamples(const Renderer::Texture& texture, bool write = false)
    {
        // This seems to work, giving downsampled mips
        // 
        // 
        // Set the downsampling shader         
        m_blitdata.getShader().bind();
        texture.bind();
        m_blitdata.getShader().setUniform2f("u_srcResolution", glm::vec2(Window::getWinWidth(), Window::getWinHeight()));
        m_blitdata.getShader().setUniform1i("u_texture", 0);

        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++) 
        
        {
            BloomMip& mip = *m_mipChain[bloomPass];




            // Set fbo target to new mips texture 

            Renderer::FrameBufferObject::setViewport((unsigned int)mip.resolution.x, (unsigned int)mip.resolution.y);//?
            m_fbo.setTargetTexture(mip.texture);


            // blit
            if (bloomPass == 0) {

                m_blitdata.getShader().bind();
                m_blitdata.getShader().setUniform1i("u_firstPass", 1);
                m_blitdata.getShader().unbind();

                m_fbo.bind();
                m_blitdata.doBlit(texture);
                m_fbo.unbind();
            }
                       
            else {

                m_blitdata.getShader().bind();
                m_blitdata.getShader().setUniform1i("u_firstPass", 0);
                m_blitdata.getShader().unbind();

                m_fbo.bind();
                m_blitdata.doBlit(m_mipChain.at(bloomPass-1)->texture); // get the previous mip texture
                m_fbo.unbind();

            }

            if (write) {
                std::stringstream path;
                path << "down_";
                path << bloomPass;
                path << ".png";

                Renderer::Texture::writeToFile(mip.texture, path.str());
            }

            m_blitdata.getShader().bind();
            m_blitdata.getShader().setUniform2f("u_srcResolution", mip.resolution);
            m_blitdata.getShader().unbind();

            mip.texture.bind();

        }
        
    }


    void RenderUpsamples(float filterRadius, bool write=false) 
    {

        m_blitdata.getShader().bind();

        m_blitdata.getShader().setUniform1f("u_filterRadius", filterRadius);
        m_blitdata.getShader().setUniform1i("u_texture", 0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        for (size_t bloomPass = m_mipChain.size() - 1; bloomPass > 0; bloomPass--) {


            BloomMip& mip = *m_mipChain[bloomPass];
            BloomMip& nextMip = *m_mipChain[bloomPass-1];

            mip.texture.bind();

            Renderer::FrameBufferObject::setViewportToTexture(nextMip.texture);
            m_fbo.setTargetTexture(nextMip.texture);

            m_fbo.bind();
            m_blitdata.doBlit(mip.texture);
            m_fbo.unbind();

            if (write) {

                std::stringstream path;
                path << "up_";
                path << bloomPass-1;
                path << ".png";

                Renderer::Texture::writeToFile(nextMip.texture, path.str());
            }
        }
        glDisable(GL_BLEND);


    }


};