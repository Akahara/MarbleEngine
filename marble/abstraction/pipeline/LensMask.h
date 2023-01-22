#pragma once

#include "VFX.h"
#include "Flares.h"

namespace visualEffects {

class LensMask : public VFX {
private:
	float             m_transparency;
	Renderer::Texture m_mask {"res/textures/dirtmask.png"};
	FlareManager      m_fManager;

public:
	LensMask()
		: VFX("LensMask"), m_transparency(0.5f)
	{
		setFragmentShader("res/shaders/lensmask.fs");
		m_isEnabled = true;
	}

	virtual void applyEffect(PipelineContext& context) override
	{
		context.fbo.setTargetTexture(context.targetTexture);

		context.fbo.bind();
		m_mask.bind(1);
		glm::vec3 sunPos = context.getContextParam<glm::vec3>("sunPos");
		Renderer::Camera camera = context.getContextParam<Renderer::Camera>("camera");

		m_blitData.getShader().bind();
		m_blitData.getShader().setUniform1i("u_mask", 1);
		m_blitData.getShader().setUniform3f("u_sunPos", sunPos);
		m_blitData.getShader().setUniform3f("u_camForward", camera.getForward());
		m_blitData.getShader().setUniform3f("u_camPos", camera.getPosition());
		glm::vec4 clip = camera.getViewProjectionMatrix() * glm::vec4(sunPos+camera.getPosition(), 1);
		glm::vec2 screenspace = ((clip / clip.w) + 1.F) * 0.5F;
		m_blitData.getShader().setUniform2f("u_sunScreenSpace", screenspace);
		context.originTexture.bind();
		m_blitData.doBlit();
		m_fManager.render(sunPos, camera);

		context.fbo.unbind();
	}

	virtual void onImGuiRender() override
	{
		VFX::onImGuiRender();
		if (m_isEnabled && ImGui::CollapsingHeader(m_name.c_str())) {
		    // no settings
		}
	}

	EFFECT_CLASS_TYPE(LensMaskEffect);
};

}