#pragma once


#include "VFX.h"
#include <random>

// This SSAO implementation should really be used in a deferrend rendering engine

namespace visualEffects {

	class SSAO : public VFX
	{
	private:

		Renderer::Texture m_normalBuffer;
		Renderer::Texture* m_depthBuffer;
		Renderer::Texture m_albedoBuffer;
		
		float m_zfar = 1000.f;
		float m_znear = .1f;

		Renderer::BlitPass m_depthTestBlitPass{ "res/shaders/shadows_testblitdepth.fs" };


		std::vector<glm::vec3> m_samplesKernel;

		Renderer::Texture* m_randomNoise;
		std::vector<glm::vec3> m_noise;

		Renderer::FrameBufferObject m_fbo;
		Renderer::Texture m_target{ Window::getWinWidth(), Window::getWinHeight() };

		
	public:
		SSAO()
			: VFX("SSAO")
		{
			m_isEnabled = true;
			/*
			m_blitData.setShader("res/shaders/SSAO.fs");
			m_blitData.getShader().bind();
			m_blitData.getShader().setUniform1i("u_texture", 0);
			m_blitData.getShader().setUniform1i("u_depth", 1);
			m_blitData.getShader().unbind();

			// Init the kernel
			std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
			std::default_random_engine generator;
			for (unsigned int i = 0; i < 64; ++i)
			{
				glm::vec3 sample(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator)
				);
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = (float)i / 64.0;
				scale = Mathf::lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				m_samplesKernel.push_back(sample);
			}


			for (unsigned int i = 0; i < 16; i++)
			{
				glm::vec3 noise(
					randomFloats(generator) * 2.0 - 1.0,
					randomFloats(generator) * 2.0 - 1.0,
					0.0f);
				m_noise.push_back(noise);
			}

			//m_randomNoise = &Renderer::Texture::createTextureFromData((const float*) & m_noise[0], 4, 4);
			m_fbo.setTargetTexture(m_target);


			return;
			*/
		}

		virtual void applyEffect(PipelineContext& context) override final
		{


			m_depthBuffer = &context.depthTexture;
			//context.originTexture.bind(0);
			/*
			context.depthTexture.bind(0);
			context.fbo.setTargetTexture(context.targetTexture);
			context.fbo.bind();


			m_depthTestBlitPass.doBlit();


			context.fbo.unbind();
			*/
			
		}


		void debug() {

			Renderer::renderDebugGUIQuadWithTexture(*m_depthBuffer, { 0,-0.5 }, { 0.5,0.5 });

			m_depthBuffer->bind(0);
			m_depthTestBlitPass.getShader().bind();
			m_depthTestBlitPass.getShader().setUniform1f("u_zFar", m_zfar);
			m_depthTestBlitPass.getShader().setUniform1f("u_zNear", m_znear);
			std::cout << m_zfar << std::endl;
			m_depthTestBlitPass.doBlit();


		}

		
		EFFECT_CLASS_TYPE(SSAOEffect);

	};

}