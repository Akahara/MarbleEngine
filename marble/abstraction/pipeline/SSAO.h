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


		int m_kernelSize = 64;
		float m_radius = 0.5f;
		float m_bias = 0.0025f;

		/////////////////////// Compute update

		
		Renderer::ComputeShader m_ssaoComputeShader{ "res/shaders/compute/ssao.comp", glm::vec2{ceil(Window::getWinWidth()/(16)), ceil(Window::getWinHeight()/(8))} };
		Renderer::ComputeShader m_blurComputeShader{ "res/shaders/compute/blur.comp", glm::vec2{ceil(Window::getWinWidth()/(16)), ceil(Window::getWinHeight()/(8))} };
		Renderer::Texture m_imgOutput{ 16*32.f, 9*32.f};
		Renderer::Texture m_blurredImg{ 16*32.f, 9*32.f};
		
		unsigned int m_noiseTexture;

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
			
			// Setup the compute shaders
			m_ssaoComputeShader.use();
			m_ssaoComputeShader.setUniform1i("gPosition", 3);
			m_ssaoComputeShader.setUniform1i("gNormal", 1);
			m_ssaoComputeShader.setUniform1i("texNoise", 2);
			m_ssaoComputeShader.setUniform1i("gDepth", 4);
			m_ssaoComputeShader.setUniform3fv("samples", 64, (const float*)&ssaoKernel[0]);

			m_blurComputeShader.use();
			m_blurComputeShader.setUniform1i("u_texture", 5);





		}

		Renderer::Texture* computeSSAOTexture(
			Renderer::Texture* gPos,
			Renderer::Texture* gNormal,
			Renderer::Texture* gDepth,
			Renderer::Camera& camera		
		)
		{
			// SSAO texture
			gNormal->bind(1);
			Renderer::Texture::bindFromId(m_noiseTexture, 2);
			gPos->bind(3);
			gDepth->bind(4);

			m_ssaoComputeShader.bindImage(m_imgOutput.getId());
			m_ssaoComputeShader.use();
			m_ssaoComputeShader.setUniformMat4f("projection", camera.getViewProjectionMatrix());
			m_ssaoComputeShader.dispatch();
			m_ssaoComputeShader.wait();
			
			
			// Blur image
			m_imgOutput.bind(5);
			m_blurComputeShader.bindImage(m_blurredImg.getId());
			m_blurComputeShader.use();
			m_blurComputeShader.dispatch();
			m_blurComputeShader.wait();
			

			return &m_blurredImg;
						
		}


		void onImGuiRender() 
		{
			if (ImGui::CollapsingHeader("SSAO")) {

				ImGui::Image(m_imgOutput.getId(), { 16 * 20, 9 * 20 }, { 0,1 }, { 1,0 });
				if (ImGui::SliderInt("KernelSize", &m_kernelSize, 1, 64) ||
					ImGui::SliderFloat("Radius", &m_radius, 0.F, 1.F) ||
					ImGui::DragFloat("Bias", &m_bias, 0.005f))

				{
					m_ssaoComputeShader.use();
					m_ssaoComputeShader.setUniform1i("kernelSize", m_kernelSize);
					m_ssaoComputeShader.setUniform1f("radius", m_radius);
					m_ssaoComputeShader.setUniform1f("bias", m_bias);



				}
			}
		
		}
		

	};

}