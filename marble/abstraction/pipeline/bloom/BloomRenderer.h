#pragma once

#include "../../Shader.h"
#include "../../ComputeShader.h"
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


    Renderer::ComputeShader m_upComputeShader{ "res/shaders/compute/bloom_up.comp", glm::vec2{ceil(Window::getWinWidth() / (16)), ceil(Window::getWinHeight() / (8))} };
    Renderer::ComputeShader m_downComputeShader{ "res/shaders/compute/bloom_down.comp", glm::vec2{ceil(Window::getWinWidth() / (16)), ceil(Window::getWinHeight() / (8))} };

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
        
        m_downComputeShader.use();
        m_downComputeShader.setUniform1i("u_texture", 0);


        m_upComputeShader.use();
        m_upComputeShader.setUniform1i("u_texture", 1);
        


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
        /*
        m_blitDown.getShader().bind();
        texture.bind();
        m_blitDown.getShader().setUniform2f("u_srcResolution", glm::vec2(Window::getWinWidth(), Window::getWinHeight()));
        m_blitDown.getShader().setUniform1i("u_texture", 0);

        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++) 
        {
            BloomMip& mip = *m_mipChain[bloomPass];

            // Set fbo target to new mips texture 

            Renderer::FrameBufferObject::setViewport((unsigned int)mip.resolution.x, (unsigned int)mip.resolution.y);
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

                // -- temp





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
            */

        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++)
        {

            BloomMip& mip = *m_mipChain[bloomPass];

            m_downComputeShader.use();
            m_downComputeShader.bindImage(mip.texture.getId());

            if (bloomPass == 0) {
                m_downComputeShader.setUniform1i("u_firstPass", 1);
                texture.bind(0); // for first pass, we use the context texture as sampler
                //m_downComputeShader.dispatch();
                m_downComputeShader.setUniform2f("u_srcResolution", mip.resolution);
                glDispatchCompute(ceil(mip.resolution.x/8.F), ceil(mip.resolution.y/4.f), 1);
                m_downComputeShader.wait();
                m_downComputeShader.setUniform1i("u_firstPass", 0);
                continue;
            }

            m_mipChain.at(bloomPass - 1)->texture.bind(0); // get the previous mip texture
            m_downComputeShader.setUniform1i("u_firstPass", 0);
            m_downComputeShader.setUniform2f("u_srcResolution", mip.resolution);
            glDispatchCompute(ceil(mip.resolution.x/8.F), ceil(mip.resolution.y/4), 1);
            m_downComputeShader.wait();
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