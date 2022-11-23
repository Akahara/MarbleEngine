#pragma once
#include "VFX.h"

namespace visualEffects {

	class DepthOfField : public VFX
	{

	private:


	public:


		DepthOfField()
			: VFX("DOF")
		{
		}

		virtual void onImGuiRender() override {

			VFX::onImGuiRender();
			if (m_isEnabled) {
				if (ImGui::CollapsingHeader(m_name.c_str())) {

				}
			}

		}

		EFFECT_CLASS_TYPE(DOFEffect);


	};
}