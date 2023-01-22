#pragma once

#include <string>
#include <filesystem>

#include "../../vendor/imgui/imgui.h"

#include "../Shader.h"
#include "../Window.h"
#include "../Texture.h"
#include "../UnifiedRenderer.h"
#include "../FrameBufferObject.h"


namespace visualEffects {


struct PipelineContext {
public:
	Renderer::FrameBufferObject fbo;

	Renderer::Texture targetTexture;
	Renderer::Texture originTexture;

	Renderer::Texture depthTexture;
	
	std::map<std::string, void*> m_params;

	template<typename T>
	T& getContextParam(const std::string& key) {
		return *(T*)m_params[key];
	}
};

#define EFFECT_CLASS_TYPE(type) virtual EffectType getType() const override {return EffectType::##type;}\

// the order in which effects are declared here is the same as
// the order they will be applied in
enum EffectType {
	na=-1,

	BloomEffect,
	DOFEffect,
	ContrastEffect,
	SaturationEffect,
	SharpnessEffect,
	SBFEffect,
	GammaCorrectionEffect,
	//TonemapperEffect, // not yet implemented
	LensMaskEffect,
	
	end_marker
};


/* Parent class for all effects */
class VFX {
protected:
	bool                m_isEnabled;
	std::string         m_name;
	
	Renderer::BlitPass  m_blitData;
	std::string         m_shaderpath; // debugging purposes

public:
	VFX()
	  : m_isEnabled(false)
	{}

	VFX(const std::string& name = "N/A")
		: m_name(name)
		, m_isEnabled(false)
	{}

	void setFragmentShader(const std::filesystem::path& fs) {
		m_blitData.setShader(fs);
		m_shaderpath = fs.string();
	}

	virtual void applyEffect(PipelineContext& context) {
		context.fbo.setTargetTexture(context.targetTexture);
		context.fbo.bind();
		context.originTexture.bind(0);
		m_blitData.doBlit();
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