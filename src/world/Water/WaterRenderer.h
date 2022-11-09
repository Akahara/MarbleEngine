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

	const float WAVE_SPEED = 0.05f;

private:

	Renderer::FrameBufferObject m_waterRefractionFbo;
	Renderer::FrameBufferObject m_waterReflectionFbo;


	Renderer::Texture m_refractionTexture{ Window::getWinWidth(), Window::getWinHeight() }; // what u see under water
	Renderer::Texture m_reflectionTexture{ Window::getWinWidth(), Window::getWinHeight() }; // what is reflected
	Renderer::Texture m_depthTexture = Renderer::Texture::createDepthTexture( Window::getWinWidth(), Window::getWinHeight() );

	Renderer::Texture m_dudvTexture = Renderer::Texture("res/textures/dudvWater.png"); //distortion
	Renderer::Texture m_normalTexture = Renderer::Texture("res/textures/waterNormal.png"); //normal

	Renderer::Shader m_waterShader = Renderer::loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/water.fs");

	RendererState m_state = NOT_READY;

	float m_moveFactor = 0;

public:


	void updateMoveFactor(float deltaTime) {

		m_moveFactor += WAVE_SPEED * deltaTime;
		m_moveFactor = std::fmod(m_moveFactor, 1.f);
		m_waterShader.bind();
		m_waterShader.setUniform1f("u_moveFactor", m_moveFactor);

	}

	// -------- Reflection

	void setupCameraForReflection(Renderer::Camera& cam, const WaterSource& source) {

		// compute distance to ground

		float distance = (cam.getPosition().y - source.getHeight()) * 2;
		std::cout << distance << std::endl;

		// place camera
		glm::vec3 camPosition = cam.getPosition();
		camPosition.y -= distance;
		cam.setPosition(camPosition);
		std::cout << "reflect cam"  << cam.getPosition().y << std::endl;
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
		Renderer::FrameBufferObject::setViewportToTexture(m_reflectionTexture);
	}



	//--------- Refraction

	void bindRefractionBuffer() {

		m_waterRefractionFbo.setTargetTexture(m_refractionTexture);
		m_waterRefractionFbo.setDepthTexture(m_depthTexture);
		m_waterRefractionFbo.bind();
		Renderer::FrameBufferObject::setViewportToTexture(m_refractionTexture);

	}

	void unbind() {
		//Renderer::getStandardMeshShader().setUniform1i()
		Renderer::FrameBufferObject::unbind();
	}


	/* TO BE CALLED AFTER BINDING THE VFX PIPELINE AND THE SCENE HAS BEEN RENDERED IN THE WATER FBO*/
	void onRenderWater(std::vector<WaterSource*> waterSources, const Renderer::Camera& camera) {



		m_waterShader.bind();
		m_reflectionTexture.bind(1);
		m_refractionTexture.bind(2);
		m_dudvTexture.bind(3);
		m_normalTexture.bind(4);
		m_depthTexture.bind(5);
		
		m_waterShader.setUniform3f("u_camPos", camera.getPosition());
		m_waterShader.setUniform1i("u_ReflectionTexture", 1);
		m_waterShader.setUniform1i("u_RefractionTexture", 2);
		m_waterShader.setUniform1i("u_dudvMap", 3);
		m_waterShader.setUniform1i("u_normalMap", 4);
		m_waterShader.setUniform1i("u_depthMap", 5);
		
		glDisable(GL_CULL_FACE);
		glEnable(GL_CLIP_DISTANCE0);

		for (WaterSource* source : waterSources) {

			drawWaterSource(*source, camera);

		}


		glEnable(GL_CULL_FACE);

		/*
		Renderer::renderDebugGUIQuadWithTexture(m_refractionTexture, { -0.75f, -0.75f }, { 0.5f, 0.5f });
		Renderer::renderDebugGUIQuadWithTexture(m_reflectionTexture, { +0.25f, -0.75f }, { 0.5f, 0.5f });
		*/

	}

	// todo finish
	template<RendererState T>
	void isInState() { assert(m_state == T); }

	void writeTexture() {

		Renderer::Texture::writeToFile(m_reflectionTexture, "reflection.png");
		Renderer::Texture::writeToFile(m_refractionTexture, "refraction.png");
	}


	void drawWaterSource(const WaterSource& source, const Renderer::Camera& camera) {

		glm::mat4 M(1.f);
		M = glm::translate(M, { source.getPosition().x, source.getHeight(), source.getPosition().y });
		M = glm::scale(M, glm::vec3(source.getSize()));

		m_waterShader.bind();
		m_waterShader.setUniformMat4f("u_M", M);
		m_waterShader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());

		source.draw();


	}



};

