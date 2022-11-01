#pragma once
#include "VFX.h"

namespace visualEffects {



	class GammaCorrection : public VFX
	{

	private:

		float m_gamma;

	public:


		GammaCorrection()
			: VFX("Gamma correction")
		{
			m_gamma = 2.2f;
			m_blitData.getShader().setUniform1f("u_gamma", m_gamma);
		}

		virtual void onImGuiRender() override {

			VFX::onImGuiRender();
			if (m_isEnabled) {
				if (ImGui::CollapsingHeader(m_name.c_str())) {

					if (ImGui::SliderFloat("Gamma", &m_gamma, 0.01f, 3.f)) {
						m_blitData.getShader().bind();
						m_blitData.getShader().setUniform1f("u_gamma", m_gamma);
					}

				}
			}
		}
		EFFECT_CLASS_TYPE(GammaCorrectionEffect);


	};
}