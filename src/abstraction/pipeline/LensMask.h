#pragma once
#include "VFX.h"

namespace visualEffects {

	class LensMask : public VFX
	{

	private:

		float m_transparency;
		Renderer::Texture m_mask {"res/textures/dirtmask.png"};
		Renderer::Texture m_flare {"res/textures/flare.png"};

	public:


		LensMask()
			: VFX("LensMask")
		{
			setFragmentShader("res/shaders/lensmask.fs");
		}

		virtual void applyEffect(PipelineContext& context) override {


			
			context.fbo.setTargetTexture(context.targetTexture);

			context.fbo.bind();
			m_mask.bind(1);
			m_flare.bind(2);
			glm::vec3 sunPos = context.getContextParam<glm::vec3>("sunPos");
			Renderer::Camera camera = context.getContextParam<Renderer::Camera>("camera");



			m_blitData.getShader().bind();
			m_blitData.getShader().setUniform1i("u_mask", 1);
			m_blitData.getShader().setUniform1i("u_flare", 2);
			m_blitData.getShader().setUniform3f("u_sunPos", sunPos);
			m_blitData.getShader().setUniform3f("u_camForward", camera.getForward());
			glm::vec4 clip = camera.getViewProjectionMatrix() * glm::vec4(sunPos+camera.getPosition(), 1);
			glm::vec2 screenspace = ((clip / clip.w) + 1.F) * 0.5F;
			m_blitData.getShader().setUniform2f("u_sunScreenSpace", screenspace);
			m_blitData.doBlit(context.originTexture);

			context.fbo.unbind();

		}

		virtual void onImGuiRender() override {

			VFX::onImGuiRender();
			if (m_isEnabled) {
				if (ImGui::CollapsingHeader(m_name.c_str())) {
				}
			}

		}

		EFFECT_CLASS_TYPE(LensMaskEffect);


	};
}