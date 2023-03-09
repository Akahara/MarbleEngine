#pragma once


#include "VFX.h"
#include "../Texture.h"
#include <random>

// This SSAO implementation should really be used in a deferrend rendering engine

namespace visualEffects {

	class SSAO : public VFX
	{
	private:

		Renderer::FrameBufferObject m_fbo;
		Renderer::Texture m_ssaoTexture{Window::getWinWidth(), Window::getWinHeight()};
		Renderer::BlitPass m_combinePass; // todo set shader
		
		//Renderer::FrameBufferObject m_fbo;

		std::vector<glm::vec3> m_samplesKernel;
		std::shared_ptr<Renderer::Texture> m_randomNoise;


		unsigned int m_depthId;
		unsigned int m_normalId;
		unsigned int m_positionId;
		std::vector<glm::vec3> m_noise;

		unsigned int noiseTexture;

		int m_kernelSize = 16;
		float m_radius = 0.5f;
		float m_bias = 0.025f;
		
		bool firstPass = true;

	public:
		SSAO()
			: VFX("SSAO")
		{
			m_isEnabled = true;
			
			m_fbo.setTargetTexture(m_ssaoTexture);
			// Init the kernel
			
				// ----------------------
			std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
			std::default_random_engine generator;
			std::vector<glm::vec3> ssaoKernel;
			for (unsigned int i = 0; i < 64; ++i)
			{
				glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = float(i) / 64.0f;

				// scale samples s.t. they're more aligned to center of kernel
				scale = Mathf::lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				ssaoKernel.push_back(sample);
			}

			// generate noise texture
			// ----------------------
			std::vector<glm::vec3> ssaoNoise;
			for (unsigned int i = 0; i < 16; i++)
			{
				glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
				ssaoNoise.push_back(noise);
			}
			glGenTextures(1, &noiseTexture);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			

			// Set-up the shader
			m_blitData.setShader("res/shaders/SSAO.fs");
			m_blitData.getShader().bind();
			m_blitData.getShader().setUniform1i("gPosition", 0);
			m_blitData.getShader().setUniform1i("gNormal", 1);
			m_blitData.getShader().setUniform1i("texNoise", 2);

			m_blitData.getShader().setUniform3fv("samples", 64, (const float*)&ssaoKernel[0]);

			m_blitData.getShader().unbind();
			

			// Set up final combine pass
			// todo
			m_combinePass.setShader("res/shaders/blur.fs");
			m_combinePass.getShader().bind();
			m_combinePass.getShader().setUniform1i("u_target", 0);
			m_combinePass.getShader().setUniform1i("u_texture", 1);
			m_combinePass.getShader().unbind();


		}

		virtual void applyEffect(PipelineContext& context) override final
		{
			
			if (firstPass) {

				m_normalId = context.getContextParam<unsigned int>("normalMapId");
				m_depthId = context.getContextParam<unsigned int>("depthMapId");
				m_positionId = context.getContextParam<unsigned int>("positionMapId");
				firstPass = false;
			}

			Renderer::Camera& camera = context.getContextParam<Renderer::Camera>("camera");
			m_blitData.getShader().bind();
			m_blitData.getShader().setUniformMat4f("projection", camera.getViewProjectionMatrix());
			m_blitData.getShader().unbind();

			Renderer::Texture::bindFromId(m_positionId, 0);
			Renderer::Texture::bindFromId(m_normalId, 1);
			Renderer::Texture::bindFromId(noiseTexture, 2);

			// Create the ssao texture
			Renderer::clear();
			m_fbo.bind();
			m_blitData.doBlit();
			m_fbo.unbind();

			// Combine ssao and actual target
			m_ssaoTexture.bind(1);
			context.fbo.setTargetTexture(context.targetTexture);
			context.originTexture.bind();
			context.fbo.bind();
			m_combinePass.doBlit(); // TODO
			context.fbo.unbind();
			
						
		}

		virtual void onImGuiRender() override 
		{
			VFX::onImGuiRender();
			if (m_isEnabled && ImGui::CollapsingHeader(m_name.c_str())) {

				ImGui::Image(m_ssaoTexture.getId(), { 16 * 20, 9 * 20 }, { 0,1 }, { 1,0 });
				if (ImGui::SliderInt("KernelSize", &m_kernelSize, 1, 64) ||
					ImGui::SliderFloat("Radius", &m_radius, 0.F, 1.F) ||
					ImGui::DragFloat("Bias", &m_bias, 0.005f))

				{
					m_blitData.getShader().bind();
					m_blitData.getShader().setUniform1i("kernelSize", m_kernelSize);
					m_blitData.getShader().setUniform1f("radius", m_radius);
					m_blitData.getShader().setUniform1f("bias", m_bias);

				}
			}
		
		}
		
		EFFECT_CLASS_TYPE(SSAOEffect);

	};

}