#pragma once

#include "WaterSource.h"

#include "../../abstraction/Texture.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"
#include "../../abstraction/Camera.h"

#include "../../abstraction/Window.h"



#include <vector>

class WaterRenderer
{
	// make sur that the state is ready for rendring
	enum RendererState {
		NOT_READY, // if both texture arent initialised and bound has not been called
		BOUND, // idk
		READY // both texture are ok
	};

private:

	Renderer::FrameBufferObject m_waterRefractionFbo;
	Renderer::FrameBufferObject m_waterReflectionFbo;

	Renderer::Texture m_refractionTexture; // what u see under water
	Renderer::Texture m_reflectionTexture; // what is reflected

	Renderer::Shader m_waterShader;


public:

	// -------- Reflection

	void bindReflectionBuffer() {

		m_waterReflectionFbo.bind();
		m_waterReflectionFbo.setTargetTexture(m_reflectionTexture);
		Renderer::FrameBufferObject::setViewport(Window::getWinWidth(), Window::getWinHeight());
	}


	//--------- Refraction

	void bindRefractionBuffer() {

		m_waterRefractionFbo.bind();

		m_waterRefractionFbo.setTargetTexture(m_refractionTexture, 0);

		Renderer::FrameBufferObject::setViewport(Window::getWinWidth(), Window::getWinHeight());
	}

	void unbind() {
		Renderer::FrameBufferObject::unbind();
	}


	/* TO BE CALLED AFTER BINDING THE VFX PIPELINE AND THE SCENE HAS BEEN RENDERED IN THE WATER FBO*/
	void onRenderWater(std::vector<WaterSource> waterSources, const Renderer::Camera& camera) {

		for (auto& source : waterSources) {

			source.draw(camera);

		}

	}
	



};

