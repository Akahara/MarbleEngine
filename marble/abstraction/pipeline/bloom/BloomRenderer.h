#pragma once

#include "../../Shader.h"
#include "../../UnifiedRenderer.h"


class BloomRenderer {
public:
    struct BloomMip {

        glm::vec2 resolution;
        Renderer::Texture texture;

        BloomMip(const glm::vec2& res)
          : resolution(res), texture(Renderer::Texture((unsigned int)res.x, (unsigned int)res.y))
        { }
    };

private:
    const unsigned int m_numberOfMips = 5;

    std::vector<BloomMip*>          m_mipChain;
    Renderer::FrameBufferObject     m_fbo;

    Renderer::BlitPass              m_blitDown; //draw in the texture
    Renderer::BlitPass              m_blitUp; //draw in the texture

public:
    BloomRenderer() 
    {  

        m_blitDown.setShader("res/shaders/bloom/downsampling.fs");
        m_blitUp.setShader("res/shaders/bloom/upsampling.fs");
        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++)
        {
            // Generate a mip with half the resolution of the previous mip
            double coef = std::pow(2, bloomPass+1);
            BloomMip* mip = new BloomMip{ glm::vec2(Window::getWinWidth() / coef, Window::getWinHeight() / coef) };
            m_mipChain.emplace_back(mip);
        }
    }

    void RenderBloomTexture(const Renderer::Texture& texture, float filterRadius, bool write = false)
    {
        m_fbo.bind();
        RenderDownsamples(texture, write);
        RenderUpsamples(filterRadius, write);
        m_fbo.unbind();
    }

    Renderer::Texture* getFinalBloomTexture() const { return &m_mipChain[0]->texture; }

private:
    void RenderDownsamples(const Renderer::Texture& texture, bool write = false)
    {
        // This seems to work, giving downsampled mips
        // 
        // Set the downsampling shader         
        m_blitDown.getShader().bind();
        texture.bind();
        m_blitDown.getShader().setUniform2f("u_srcResolution", glm::vec2(Window::getWinWidth(), Window::getWinHeight()));
        m_blitDown.getShader().setUniform1i("u_texture", 0);

        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++) 
        {
            BloomMip& mip = *m_mipChain[bloomPass];

            // Set fbo target to new mips texture 

            Renderer::FrameBufferObject::setViewport((unsigned int)mip.resolution.x, (unsigned int)mip.resolution.y);//?
            m_fbo.setTargetTexture(mip.texture);

            // blit
            if (bloomPass == 0) {

                m_blitDown.getShader().bind();
                m_blitDown.getShader().setUniform1i("u_firstPass", 1);
                m_blitDown.getShader().unbind();

                m_fbo.bind();
                texture.bind(0);
                m_blitDown.doBlit();
                m_fbo.unbind();
            } else {

                m_blitDown.getShader().bind();
                m_blitDown.getShader().setUniform1i("u_firstPass", 0);
                m_blitDown.getShader().unbind();

                m_fbo.bind();
                m_mipChain.at(bloomPass - 1)->texture.bind(); // get the previous mip texture
                m_blitDown.doBlit();
                m_fbo.unbind();
            }

            if (write) {
                std::stringstream path;
                path << "down_" << bloomPass << ".png";

                Renderer::Texture::writeToFile(mip.texture, path.str());
            }

            m_blitDown.getShader().bind();
            m_blitDown.getShader().setUniform2f("u_srcResolution", mip.resolution);
            m_blitDown.getShader().unbind();

            mip.texture.bind();
        }
    }


    void RenderUpsamples(float filterRadius, bool write=false) 
    {
        m_blitUp.getShader().bind();
        m_blitUp.getShader().setUniform1f("u_filterRadius", filterRadius);
        m_blitUp.getShader().setUniform1i("u_texture", 0);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD); // TODO FIX pin down the inclusing of glad.h that allows this call
   
                                      // glad.h should not be included in any .h file

        for (size_t bloomPass = m_mipChain.size() - 1; bloomPass > 0; bloomPass--) {
            BloomMip& mip = *m_mipChain[bloomPass];
            BloomMip& nextMip = *m_mipChain[bloomPass-1];

            mip.texture.bind();

            Renderer::FrameBufferObject::setViewportToTexture(nextMip.texture);
            m_fbo.setTargetTexture(nextMip.texture);

            m_fbo.bind();
            mip.texture.bind(0);
            m_blitUp.doBlit();
            m_fbo.unbind();

            if (write) {
                std::stringstream path;
                path << "up_";
                path << bloomPass-1;
                path << ".png";

                Renderer::Texture::writeToFile(nextMip.texture, path.str());
            }
        }

        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
};