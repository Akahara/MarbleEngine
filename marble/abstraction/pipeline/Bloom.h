#pragma once
 
#include "bloom/BloomFBO.h"
#include "bloom/BloomRenderer.h"

#include "VFX.h"

namespace visualEffects {

class Bloom : public VFX
{
private:
	Renderer::BlitPass          m_blitFinal;
	BloomRenderer               m_renderer;
	Renderer::FrameBufferObject m_fboBloom;
	Renderer::Texture           m_depth = Renderer::Texture::createDepthTexture(Window::getWinWidth(), Window::getWinHeight());
	
	float m_filterRadius = 0.005f;
	float m_exposure = 3.2f;
	float m_strenght = 0.8f;

public:
	Bloom()
		: VFX("Bloom")
	{
		m_isEnabled = true;

		m_blitFinal.setShader("res/shaders/bloom/finalBloom.fs");

		m_blitFinal.getShader().bind();
		m_blitFinal.getShader().setUniform1i("u_sceneTexture", 0);
		m_blitFinal.getShader().setUniform1i("u_finalBloom", 1);
		m_blitFinal.getShader().setUniform1f("u_bloomStrength", m_strenght);
		m_blitFinal.getShader().setUniform1f("u_exposure", m_exposure);
		m_blitFinal.getShader().unbind();

		m_fboBloom.setDepthTexture(m_depth);
	}

	virtual void applyEffect(PipelineContext& context) override final
	{
		context.fbo.setTargetTexture(context.targetTexture);

		m_renderer.RenderBloomTexture(context.originTexture, m_filterRadius, false);

		Renderer::Texture* finalTexture = m_renderer.getFinalBloomTexture();

		Renderer::FrameBufferObject::setViewportToTexture(context.targetTexture);

		finalTexture->bind(1);

		context.fbo.bind();
		context.originTexture.bind();
		m_blitFinal.doBlit();
		context.fbo.unbind();
	}

	virtual void onImGuiRender() override
	{
		VFX::onImGuiRender();
		if (m_isEnabled && ImGui::CollapsingHeader(m_name.c_str())) {
			ImGui::SliderFloat("bloomRadius", &m_filterRadius, 0.f, 0.1f);

			//------//

			if (ImGui::SliderFloat("bloomStrength", &m_strenght, 0, 5)) {
				m_blitFinal.getShader().bind();
				m_blitFinal.getShader().setUniform1f("u_bloomStrength", m_strenght);
				m_blitFinal.getShader().unbind();
			}

			//--//

			if (ImGui::SliderFloat("Exposure", &m_exposure, 0, 5)) {
				m_blitFinal.getShader().bind();
				m_blitFinal.getShader().setUniform1f("u_exposure", m_exposure);
				m_blitFinal.getShader().unbind();
			}
		}
	}

	EFFECT_CLASS_TYPE(BloomEffect);

};

}