#pragma once

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

    Renderer::ComputeShader m_upComputeShader{ "res/shaders/compute/bloom_up.comp", glm::vec2{ceil(Window::getWinWidth() / (16)), ceil(Window::getWinHeight() / (8))} };
    Renderer::ComputeShader m_downComputeShader{ "res/shaders/compute/bloom_down.comp", glm::vec2{ceil(Window::getWinWidth() / (16)), ceil(Window::getWinHeight() / (8))} };

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
        
        m_downComputeShader.use();
        m_downComputeShader.setUniform1i("u_texture", 0);


        m_upComputeShader.use();
        m_upComputeShader.setUniform1i("u_texture", 0);
        


    }

    Renderer::Texture* RenderBloomTexture(const Renderer::Texture& texture, float filterRadius, bool write = false)
    {
        RenderDownsamples(texture);
        RenderUpsamples(filterRadius);
        return &m_mipChain[0]->texture;
    }

private:

    /**
    * Fills the mip chain with downsampled textures of the given parameter.
    */
    void RenderDownsamples(const Renderer::Texture& texture)
    {

        for (unsigned int bloomPass = 0; bloomPass < m_numberOfMips; bloomPass++)
        {

            BloomMip& mip = *m_mipChain[bloomPass];

            m_downComputeShader.use();
            m_downComputeShader.bindImage(mip.texture.getId());

            if (bloomPass == 0) {
                texture.bind(0);                                    // for first pass, we use the context texture as sampler
                m_downComputeShader.setUniform1i("u_firstPass", 1); // allows for ectraction of hdr pixels

                m_downComputeShader.setUniform2f("u_srcResolution", mip.resolution);
                // execute the compute shader
                glDispatchCompute(ceil(mip.resolution.x/8.F), ceil(mip.resolution.y/4.f), 1);
                m_downComputeShader.wait();
                m_downComputeShader.setUniform1i("u_firstPass", 0);
                continue;
            }

            m_mipChain.at(bloomPass - 1)->texture.bind(0); // get the previous mip texture
            m_downComputeShader.setUniform2f("u_srcResolution", mip.resolution);
            glDispatchCompute(ceil(mip.resolution.x/8.F), ceil(mip.resolution.y/4), 1);
            m_downComputeShader.wait();
            mip.texture.bind();
        }
    }

    /**
    * 
    * Overwrites the textures of each mip to be a sum of the previous mip and the current one, giving a
    * blurred sum of the texture in the previous mip resolution.
    */

    void RenderUpsamples(float filterRadius) 
    {


        m_upComputeShader.use();
        m_upComputeShader.setUniform1f("u_filterRadius", filterRadius);

        for (size_t bloomPass = m_mipChain.size() - 1; bloomPass > 0; bloomPass--) {

            BloomMip& mip = *m_mipChain[bloomPass];
            BloomMip& nextMip = *m_mipChain[bloomPass - 1];


            m_upComputeShader.bindImage(nextMip.texture.getId());
            mip.texture.bind(0);
            glDispatchCompute(ceil(nextMip.resolution.x / 8.F), ceil(nextMip.resolution.y / 4), 1);
            m_upComputeShader.wait();


        }

    }
};