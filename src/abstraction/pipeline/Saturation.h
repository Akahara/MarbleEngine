#pragma once
#include "VFX.h"

namespace visualEffects {



	class Saturation : public VFX 
	{

	private:


	public:

		EFFECT_CLASS_TYPE(SaturationEffect);

		Saturation()
			: VFX("Saturation")
		{
		}
		

	};
}