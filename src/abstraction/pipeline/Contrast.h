#pragma once
#include "VFX.h"

namespace visualEffects {

	class Contrast : public VFX
	{

	private:

		float m_shift;

	public:


		Contrast()
			: VFX("Contrast")
		{
			setFragmentShader("res/shaders/contrast.fs");
			m_shift = 0.f;

			m_blitData.getShader().bind();
			m_blitData.getShader().setUniform1f("u_contrastShift", m_shift);
			m_blitData.getShader().unbind();


		}

		virtual void onImGuiRender() override {

			VFX::onImGuiRender();
			if (m_isEnabled) {
				if (ImGui::CollapsingHeader(m_name.c_str())) {
					if (ImGui::SliderFloat("shift", &m_shift, 0.f, 2.f)) {
						m_blitData.getShader().bind();
						m_blitData.getShader().setUniform1f("u_contrastShift", m_shift);
					}
				}
			}

		}

		EFFECT_CLASS_TYPE(ContrastEffect);


	};
}