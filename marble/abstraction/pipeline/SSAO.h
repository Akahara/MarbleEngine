#pragma once


#include "VFX.h"
#include "../Texture.h"
#include "../ComputeShader.h"
#include <random>
#include <glm/vec2.hpp>

// This SSAO implementation should really be used in a deferrend rendering engine
// this has to be done before lighting pass, so its not really a vfx

namespace visualEffects {

	class SSAO
	{
	private:

		Renderer::FrameBufferObject m_ssaoFBO;
		Renderer::Texture m_ssaoTexture{Window::getWinWidth(), Window::getWinHeight()};
		Renderer::Texture m_blurredTexture{Window::getWinWidth(), Window::getWinHeight()};
		

		unsigned int m_noiseTexture;

		int m_kernelSize = 16;
		float m_radius = 0.25f;
		float m_bias = 0.25f;
		

		Renderer::BlitPass m_ssaoPass;

		Renderer::BlitPass m_blurPass; // todo set shader
		Renderer::FrameBufferObject m_blurFBO;



		/////////////////////// Compute update

		
		Renderer::ComputeShader m_ssaoComputeShader{ "res/shaders/compute/ssao.comp", glm::vec2{8,4} };
		Renderer::Texture m_imgOutput{ Window::getWinWidth(), Window::getWinHeight() };




	public:
		SSAO() 
		{
			
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
			glGenTextures(1, &m_noiseTexture);
			glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			


			// Set-up the shader
			m_ssaoPass.setShader("res/shaders/SSAO.fs");
			m_ssaoPass.getShader().bind();
			m_ssaoPass.getShader().setUniform1i("gPosition", 0);
			m_ssaoPass.getShader().setUniform1i("gNormal", 1);
			m_ssaoPass.getShader().setUniform1i("texNoise", 2);
			m_ssaoPass.getShader().setUniform3fv("samples", 64, (const float*)&ssaoKernel[0]);
			m_ssaoPass.getShader().unbind();
			

			// Set up the blur pass
			m_blurPass.setShader("res/shaders/blur.fs");
			m_blurPass.getShader().bind();
			m_blurPass.getShader().setUniform1i("u_texture", 0);
			m_blurPass.getShader().unbind();


			// Setup the fbos
			m_ssaoFBO.setTargetTexture(m_ssaoTexture);
			m_blurFBO.setTargetTexture(m_blurredTexture);

			m_ssaoComputeShader.use();
			m_ssaoComputeShader.setUniform1i("gPosition", 3);
			m_ssaoComputeShader.setUniform1i("gNormal", 1);
			m_ssaoComputeShader.setUniform1i("texNoise", 2);
			m_ssaoComputeShader.setUniform3fv("samples", 64, (const float*)&ssaoKernel[0]);






		}

		Renderer::Texture* computeSSAOTexture(
			Renderer::Texture* gPos,
			Renderer::Texture* gNormal,
			Renderer::Camera& camera		
		)
		{

			gPos->bind(3);
			gNormal->bind(1);
			Renderer::Texture::bindFromId(m_noiseTexture, 2);
			m_ssaoComputeShader.bindImage(m_imgOutput.getId());
			m_ssaoComputeShader.use();
			m_ssaoComputeShader.setUniformMat4f("projection", camera.getViewProjectionMatrix());

			glDispatchCompute(ceil(Window::getWinWidth() / 8), ceil(Window::getWinHeight() / 4), 1);
			m_ssaoComputeShader.wait();

			
			/*
			gPos->bind(0);
			gNormal->bind(1);
			Renderer::Texture::bindFromId(m_noiseTexture, 2);
			m_ssaoPass.getShader().bind();
			m_ssaoPass.getShader().setUniformMat4f("projection", camera.getViewProjectionMatrix());
			m_ssaoPass.getShader().unbind();
		
			// Create the ssao texture
			m_ssaoFBO.bind();
			m_ssaoPass.doBlit();
			m_ssaoFBO.unbind();
			*/

			// Blur the ssao, now ssaoTexture is complete
			m_imgOutput.bind(0);
			m_blurFBO.bind();
			m_blurPass.doBlit();
			m_blurFBO.unbind();
			
			return &m_blurredTexture;
						
		}


		void onImGuiRender() 
		{
			if (ImGui::CollapsingHeader("SSAO")) {

				ImGui::Image(m_ssaoTexture.getId(), { 16 * 20, 9 * 20 }, { 0,1 }, { 1,0 });
				ImGui::Image(m_imgOutput.getId(), { 16 * 20, 9 * 20 }, { 0,1 }, { 1,0 });
				if (ImGui::SliderInt("KernelSize", &m_kernelSize, 1, 64) ||
					ImGui::SliderFloat("Radius", &m_radius, 0.F, 1.F) ||
					ImGui::DragFloat("Bias", &m_bias, 0.005f))

				{
					m_ssaoPass.getShader().bind();
					m_ssaoPass.getShader().setUniform1i("kernelSize", m_kernelSize);
					m_ssaoPass.getShader().setUniform1f("radius", m_radius);
					m_ssaoPass.getShader().setUniform1f("bias", m_bias);

				}
			}
		
		}
		

	};

}