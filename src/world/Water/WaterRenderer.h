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

	Renderer::Texture m_refractionTexture{ Window::getWinWidth(), Window::getWinHeight() }; // what u see under water
	Renderer::Texture m_reflectionTexture{ Window::getWinWidth(), Window::getWinHeight() }; // what is reflected

	//Renderer::Shader m_waterShader = Renderer::loadShaderFromFiles("res/shaders/water.vs", "res/shaders/water.fs");

	RendererState m_state = NOT_READY;

public:

	void bindTextures() {

		m_refractionTexture.bind(3);
		m_reflectionTexture.bind(4);


	}

	// -------- Reflection

	void setupCameraForReflection(Renderer::Camera& cam, const WaterSource &source) {

		// compute distance to ground

		float distance = (cam.getPosition().y - source.getHeight()) * 2;

		// place camera
		glm::vec3 camPosition = cam.getPosition();
		camPosition.y -= distance;
		cam.setPosition(camPosition);

		// Change view
		cam.inversePitch();



	}

	void undoSetupCameraForReflection(Renderer::Camera& cam, const WaterSource& source) {

		// compute distance to ground

		float distance = (source.getHeight() - cam.getPosition().y) * 2;

		// place camera
		glm::vec3 camPosition = cam.getPosition();
		camPosition.y += distance;
		cam.setPosition(camPosition);

		// Change view
		cam.inversePitch();


	}

	void bindReflectionBuffer() {

		m_waterReflectionFbo.setTargetTexture(m_reflectionTexture);
		m_waterReflectionFbo.bind();
		Renderer::FrameBufferObject::setViewport(Window::getWinWidth(), Window::getWinHeight());
	}



	//--------- Refraction

	void bindRefractionBuffer() {

		m_waterRefractionFbo.setTargetTexture(m_refractionTexture);
		m_waterRefractionFbo.bind();
		Renderer::FrameBufferObject::setViewport(Window::getWinWidth(), Window::getWinHeight());

	}

	void unbind() {
		//Renderer::getStandardMeshShader().setUniform1i()
		Renderer::FrameBufferObject::unbind();
	}


	/* TO BE CALLED AFTER BINDING THE VFX PIPELINE AND THE SCENE HAS BEEN RENDERED IN THE WATER FBO*/
	void onRenderWater(std::vector<WaterSource*> waterSources, const Renderer::Camera& camera) {

		
		glDisable(GL_CULL_FACE);
		glEnable(GL_CLIP_DISTANCE0);

		Renderer::getStandardMeshShader().bind();
		m_reflectionTexture.bind(5);
		m_refractionTexture.bind(6);
		//Renderer::Texture::writeToFile(m_reflectionTexture, "REFLECTIONonwaterrendeer.png");

		Renderer::getStandardMeshShader().unbind();

		for (WaterSource* source : waterSources) {

			source->draw(camera);

		}

		
		glEnable(GL_CULL_FACE);
		Renderer::renderDebugGUIQuadWithTexture(camera, m_refractionTexture, { -0.75f, -0.75f }, { 0.5f, 0.5f });
		Renderer::renderDebugGUIQuadWithTexture(camera, m_reflectionTexture, { +0.25f, -0.75f }, { 0.5f, 0.5f });



	}
	
	// todo finish
	template<RendererState T>
	void isInState() { assert(m_state==T); }

	void writeTexture() {

		Renderer::Texture::writeToFile(m_reflectionTexture, "reflection.png");
		Renderer::Texture::writeToFile(m_refractionTexture, "refraction.png");
	}



};

