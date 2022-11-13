#pragma once
#include "VFX.h"

namespace visualEffects {



	class Sharpness : public VFX
	{

	private:

		float m_amout = 0.3f;

	public:


		Sharpness()
			: VFX("Sharpness")
		{
			m_blitData.getShader().bind();
			m_blitData.getShader().setUniform1f("u_amount", m_amout);
			m_blitData.getShader().unbind();
		}

		virtual void onImGuiRender() override {
			VFX::onImGuiRender();
			if (m_isEnabled) {
				if (ImGui::CollapsingHeader(m_name.c_str())) {
					if (ImGui::SliderFloat("amount", &m_amout, 0.f, 1.f)) {
						m_blitData.getShader().bind();
						m_blitData.getShader().setUniform1f("u_amount", m_amout);
						m_blitData.getShader().unbind();
					}
				}
			}
			

		}

		EFFECT_CLASS_TYPE(SharpnessEffect);


	};
}