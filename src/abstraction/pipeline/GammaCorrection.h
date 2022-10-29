#pragma once
#include "VFX.h"

namespace visualEffects {



	class GammaCorrection : public VFX
	{

	private:


	public:

		EFFECT_CLASS_TYPE(GammaCorrectionEffect);

		GammaCorrection()
			: VFX("Gamma correction")
		{
		}


	};
}