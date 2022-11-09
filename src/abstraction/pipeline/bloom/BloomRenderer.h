#pragma once

#include "../../Shader.h"
#include "../../UnifiedRenderer.h"
#include "BloomFBO.h"


class BloomRenderer
{
public:
    BloomRenderer() {}
    void init() {
        if (m_Init) return;
        const unsigned int num_bloom_mips = 5; // Experiment with this value
        m_FBO.init( num_bloom_mips);

        // Shaders
        m_DownsampleShader = Renderer::loadShaderFromFiles("res/shaders/blit.vs", "res/shaders/bloom/downsampling.fs");
        m_UpsampleShader = Renderer::loadShaderFromFiles("res/shaders/blit.vs", "res/shaders/bloom/upsampling.fs");

        // Downsample

        m_DownsampleShader.bind();
        m_DownsampleShader.setUniform1i("u_scrTexture", 0);
        m_DownsampleShader.unbind();

        m_UpsampleShader.bind();
        m_UpsampleShader.setUniform1i("u_scrTexture", 0);
        m_UpsampleShader.unbind();
        


        m_Init = true;
    }
    void Destroy();
    void RenderBloomTexture(const Renderer::Texture& texture, float filterRadius) {

        m_FBO.bindForWriting();
        RenderDownsamples(texture);
        RenderUpsamples(filterRadius);
        Renderer::FrameBufferObject::setViewportToWindow();
    }

    Renderer::Texture& BloomTexture() { return m_FBO.getMipChain().at(0)->texture; }

private:
    void RenderDownsamples(const Renderer::Texture& texture) {

        const std::vector<BloomMip*>& mipChain = m_FBO.getMipChain();

        m_blitdata.setShader("res/shaders/bloom/downsampling.fs");

        m_DownsampleShader.bind();
        m_DownsampleShader.setUniform2f("u_scrResolution", { Window::getWinWidth(), Window::getWinHeight() });

        // Bind srcTexture (HDR color buffer) as initial texture input
        texture.bind();

        // Progressively downsample through the mip chain
        for (int i = 0; i < mipChain.size(); i++)
        {
            BloomMip& mip = *mipChain[i];
            glViewport(0, 0, mip.size.x, mip.size.y);
            m_FBO.m_FBO.setTargetTexture(mip.texture);
            m_blitdata.doBlit(mip.texture);

            // Set current mip resolution as srcResolution for next iteration
            m_DownsampleShader.bind();
            m_DownsampleShader.setUniform2f("u_scrResolution", { Window::getWinWidth(), Window::getWinHeight() });
            // Set current mip as texture input for next iteration
            mip.texture.bind();
        }

        m_DownsampleShader.unbind();
    }
    void RenderUpsamples(float filterRadius) {

        std::vector<BloomMip*>& mipChain = m_FBO.getMipChain();
        m_blitdata.setShader("res/shaders/bloom/upsampling.fs");

        m_UpsampleShader.bind();
        m_UpsampleShader.setUniform1f("u_filterRadius", filterRadius);

        // Enable additive blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        for (int i = mipChain.size() - 1; i > 0; i--)
        {
            BloomMip& mip = *mipChain[i];
            BloomMip& nextMip = *mipChain[i - 1];

            // Bind viewport and texture from where to read
            Renderer::FrameBufferObject::setViewportToTexture(mip.texture);
            mip.texture.bind();

            // Set framebuffer render target (we write to this texture)
            m_FBO.m_FBO.setTargetTexture(mip.texture);

            // Render screen-filled quad of resolution of current mip
            m_blitdata.doBlit(mip.texture);
        }

        // Disable additive blending
        //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
        glDisable(GL_BLEND);

        m_UpsampleShader.unbind();
    }

    bool                m_Init;
    BloomFBO            m_FBO;
    Renderer::Shader   m_DownsampleShader;
    Renderer::Shader   m_UpsampleShader;

    Renderer::BlitPass m_blitdata;

};