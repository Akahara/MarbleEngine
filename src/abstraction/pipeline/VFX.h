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


struct PipelineContext {


	Renderer::FrameBufferObject fbo;

	Renderer::Texture targetTexture;
	Renderer::Texture originTexture;

	Renderer::Texture depthTexture;



};

#define EFFECT_CLASS_TYPE(type) virtual EffectType getType() const override {return EffectType::##type;}\

	enum EffectType {
		na=-1,
		BloomEffect,
		ContrastEffect,
		SaturationEffect,
		SharpnessEffect,
		SBFEffect,
		GammaCorrectionEffect,
		TonemapperEffect
	} ;



class VFX {

protected:

	bool						m_isEnabled ;
	std::string					m_name;
	
	Renderer::BlitPass          m_blitData;
	std::string					m_shaderpath; // debugging purposes


public:

	VFX() : m_isEnabled(false) {

	}
	VFX(const std::string& name = "N/A")
		: m_name(name)
		, m_isEnabled(true)
		
	{}

	void setFragmentShader(const std::filesystem::path& fs) {
		m_blitData.setShader(fs);
		m_shaderpath = fs.string();
	}


	virtual void applyEffect(PipelineContext& context) {

		context.fbo.setTargetTexture(context.targetTexture);
		context.fbo.bind();
		m_blitData.doBlit(context.originTexture);
		context.fbo.unbind();

	}

	Renderer::Shader& getShader() { return m_blitData.getShader(); }

	virtual void onImGuiRender() {
		ImGui::Checkbox(m_name.c_str(), &m_isEnabled);
	}


	std::string getShaderName() const { return m_shaderpath; }
	std::string getName() const { return m_name; }

	virtual EffectType getType() const = 0;

	bool isEnabled() const { return m_isEnabled; }




};
}