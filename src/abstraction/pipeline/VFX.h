#pragma once

#include <string>
#include <filesystem>

#include "../Shader.h"
#include "../Window.h"
#include "../Texture.h"
#include "../UnifiedRenderer.h"
#include "../FrameBufferObject.h"


#include "../../vendor/imgui/imgui.h"

/* Classe mère de tous les effets */


namespace visualEffects {


#define EFFECT_CLASS_TYPE(type) virtual EffectType getType() const override {return EffectType::##type;}\

	enum EffectType {
		na=-1,
		ContrastEffect,
		SaturationEffect,
		SharpnessEffect,
		BloomEffect,
		SBFEffect,
		GammaCorrectionEffect,
		TonemapperEffect
	};



class VFX {

private:

	bool						m_isEnabled ;
	std::string					m_name;
	
	Renderer::BlitPass          m_blitData;


public:

	VFX() {

	}
	VFX(const std::string& name = "N/A")
		: m_name(name)
		, m_isEnabled(true)
		
	{}

	void setFragmentShader(const std::filesystem::path& fs) {
		m_blitData.setShader(fs);
	}

	virtual Renderer::Texture& applyEffect(Renderer::Texture& targetTexture) {

		Renderer::FrameBufferObject::setViewport(Window::getWinWidth(), Window::getWinHeight());
		m_blitData.doBlit(targetTexture);

		return targetTexture;

	}

	void onImGuiRender() const {

		ImGui::Checkbox(m_name.c_str(), (bool*) & m_isEnabled);

	}



	std::string getName() const { return m_name; }

	virtual EffectType getType() const {

		return EffectType::na;


	 }

	bool isEnabled() const { return m_isEnabled; }




};
}