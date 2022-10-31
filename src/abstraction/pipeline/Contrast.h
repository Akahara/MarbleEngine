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
			m_shift = 0.f;
			m_blitData.getShader().setUniform1f("u_contrastShift", m_shift);
		}

		virtual void onImGuiRender() override {

			VFX::onImGuiRender();
			if (ImGui::SliderFloat("shift", &m_shift, 0.f, 2.f)) {
				m_blitData.getShader().bind();
				m_blitData.getShader().setUniform1f("u_contrastShift", m_shift);
			}


		}
		EFFECT_CLASS_TYPE(ContrastEffect);


	};
}