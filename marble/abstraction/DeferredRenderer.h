#pragma once

#include <numeric>
#include <functional>

#include "FrameBufferObject.h"
#include "../World/Light/LightManager.h"
#include "UnifiedRenderer.h"
#include "pipeline/VFXPipeline.h"

#include <vector>

struct gBufferLayout {

	Renderer::Texture albedo{ Window::getWinWidth(), Window::getWinHeight() };
	Renderer::Texture normal{ Window::getWinWidth(), Window::getWinHeight() };
	Renderer::Texture position{ Window::getWinWidth(), Window::getWinHeight() };

	Renderer::Texture depth = Renderer::Texture::createDepthTexture(Window::getWinWidth(), Window::getWinHeight());
public:

	std::vector<Renderer::Texture*> getTextures() {
		return {
			&albedo, &normal,&position
		};
	}

};

struct gBuffer {


	gBufferLayout textures;
	Renderer::FrameBufferObject fbo;


	explicit gBuffer() 
	{
		// Setup the fbo according to the layout (should be automated)

		fbo.setDepthTexture(textures.depth);

		fbo.setTargetTexture(textures.albedo,0);
		fbo.setTargetTexture(textures.normal,1);
		fbo.setTargetTexture(textures.position,2);

		fbo.bind();
		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
		fbo.unbind();


	}

};


class DeferredRenderer
{

private:

	gBuffer						m_gBuffer;
	World::LightRenderer		m_lightEngine;

	Renderer::FrameBufferObject m_final;
	Renderer::Texture m_target{Window::getWinWidth(), Window::getWinHeight()};

	Renderer::BlitPass m_deferredPass;
	Renderer::BlitPass last;

	visualEffects::VFXPipeline m_vfx;
	bool m_renderPipeline = false;


	visualEffects::SSAO m_ssaoRenderer;
	bool m_renderSSAO = false;
	float whitepixel[4] = {1,1,1,1};
	Renderer::Texture m_whitePixel = Renderer::Texture::createTextureFromData(whitepixel, 1, 1, 4);

public:

	DeferredRenderer() 
	{

		// Set up the FBO correctly (depth map, and 3 textures attachments)
		m_final.setTargetTexture(m_target);

		//Set up deferred pass
		m_deferredPass.setShader("res/shaders/deferredPass.fs");
		
		int samplers[16];
		std::iota(samplers, samplers + 16, 0);

		m_deferredPass.getShader().bind();
		m_deferredPass.getShader().setUniform1iv("u_gBufferTextures", 16, samplers);
		m_deferredPass.getShader().setUniform1i("u_ssaoTexture", 16 );
		m_deferredPass.getShader().unbind();

		// Bloom setup

		
		m_vfx.registerEffect<visualEffects::Bloom>();
		m_vfx.registerEffect<visualEffects::GammaCorrection>();
		m_vfx.registerEffect<visualEffects::Saturation>();
		m_vfx.registerEffect<visualEffects::Contrast>();
		m_vfx.registerEffect<visualEffects::Sharpness>();
		m_vfx.registerEffect<visualEffects::LensMask>();
		m_vfx.sortPipeline();
		

		m_vfx.addContextParam<glm::vec3>({ 1000,1000,1000 }, "sunPos");
		m_vfx.addContextParam < Renderer::Camera >({}, "camera");




	}

	void render(const std::function<void()>& renderFn, Renderer::Camera& camera) 
	{
		computeGeometryPass(renderFn);
		combineAndApplyLights(camera);

	}

	gBufferLayout* fetchTexture() {
		
		return &m_gBuffer.textures;

	}

	unsigned int getDepthTextureId() const {
		return m_gBuffer.textures.depth.getId();
	}

	void renderImGuiDebugWindow() 
	{


		static constexpr float image_size = 20.F;
		static const ImVec2 size = { 16 * image_size, 9 * image_size };
		static bool renderBuffers = false;
		ImGui::Checkbox("Render gBuffer buffers", &renderBuffers);

				if (renderBuffers && ImGui::CollapsingHeader("gBuffer content")) {


					std::vector<Renderer::Texture*> textures = m_gBuffer.textures.getTextures();

					for (int i = 0; i < textures.size(); i++) {
						auto* t = textures[i];
						ImGui::Image(t->getId(), size, { 0,1 }, { 1,0 });
					}
						ImGui::Image(m_gBuffer.textures.depth.getId(), size, { 0,1 }, { 1,0 });
				
		}

		ImGui::Checkbox("Turn on SSAO", &m_renderSSAO);

		if (m_renderSSAO && ImGui::CollapsingHeader("SSAO preview")) {
				m_ssaoRenderer.onImGuiRender();
			
		}
		
		
		ImGui::Checkbox("Apply Pipeline", &m_renderPipeline);
		if (m_renderPipeline) m_vfx.onImGuiRender();
		m_lightEngine.onImguiRender();

	}



private:

	/* Fills the gBuffer with the scene informations */
	void computeGeometryPass(const std::function<void()>& renderFn)
	{

		if (Renderer::getCurrentRenderingState() != Renderer::RenderingState::DEFERRED) {
			Renderer::setRenderingState(Renderer::RenderingState::DEFERRED);
		}
		m_gBuffer.fbo.bind();
		renderFn();
		m_gBuffer.fbo.unbind();
		
	}

	/* Uses gBuffer data for light computing */
	void combineAndApplyLights(Renderer::Camera& camera)
	{

		// SSAO stuff
		if (m_renderSSAO) {

		Renderer::Texture* ssaoTexture =
			m_ssaoRenderer.computeSSAOTexture(
				&m_gBuffer.textures.position,
				&m_gBuffer.textures.normal,
				&m_gBuffer.textures.depth,
				camera
			);
			ssaoTexture->bind(16);// todo change this
		}
		else {
			m_whitePixel.bind(16);
		}

		// Bind all textures
		std::vector<Renderer::Texture*> textures = m_gBuffer.textures.getTextures();

		for (int i = 0; i < textures.size(); i++) {
			auto* t = textures[i];
			t->bind(i);
		}

		// Upload lights
		
		m_deferredPass.getShader().bind();
		m_deferredPass.getShader().setUniform3f("u_cameraPos", camera.getPosition());
		m_deferredPass.getShader().unbind();
		m_lightEngine.uploadLightsToShader(m_deferredPass.getShader());

		// Deferred pass, blit into target
		
		if (!m_renderPipeline) {
			m_deferredPass.doBlit();
		}
		else {

			m_final.bind();
			m_deferredPass.doBlit();
			m_final.unbind();
			applyPostEffects(camera);
		}

	}

	/* Blit to the screen */
	// TODO : combine this with vfx pipeline
	// todo : this works, but it can be better !!
	void applyPostEffects(Renderer::Camera& camera) 
	{

		m_vfx.setContextParam("camera", camera);
		m_vfx.bind();
		m_target.bind(0);
		last.doBlit();
		m_vfx.unbind();
		m_vfx.renderPipeline();

	}

	void applySSAO(bool v) { m_renderSSAO = v; }
};

