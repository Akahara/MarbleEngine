#pragma once

#include <vector> 
#include <array>
#include <functional>

#include <string>
#include <sstream>

#include <map>
#include "VFX.h"


namespace visualEffects {

	class VFXPipeline {

	private:

		struct PipelineContext {

			bool isTargetReady = false;
			Renderer::FrameBufferObject fbo;
			Renderer::Texture targetTexture;
			Renderer::Texture depthTexture;

		} m_context ;

		std::vector< VFX* > m_effects;

	public:
		

		VFXPipeline(int w, int h) {

			m_context.targetTexture = Renderer::Texture(w,h);
			m_context.depthTexture = Renderer::Texture::createDepthTexture(w,h);

			m_context.fbo.setTargetTexture(m_context.targetTexture);
			m_context.fbo.setDepthTexture(m_context.depthTexture);

		}

		void setTargetTexture(Renderer::Texture&& texture) {
				
				m_context.targetTexture = std::move(texture);
				m_context.fbo.setTargetTexture(m_context.targetTexture);
				m_context.isTargetReady = true;
			
		}


		void setShaderOfEffect(EffectType type,const std::filesystem::path& fragmentShader) {

			for (const auto& effect : m_effects) {

				if (effect->getType() == type) {

					effect->setFragmentShader(fragmentShader);

				}


			}

		}
		void bind() const {
			m_context.fbo.bind();
			Renderer::FrameBufferObject::setViewportToTexture(m_context.targetTexture);
		}

		void unbind() const {
			m_context.fbo.unbind();
		}


		template<class T> 
		void registerEffect() {

			T* ptr = new T;

			m_effects.push_back( ptr  );
			std::cout << "Registered an effect : " << ((VFX*)ptr)->getName() << std::endl;

		}



		/*
		Sorts the effect list according to the defined order in VFH.h
		*/ 
		
		void sortPipeline() {

			std::array<int, 7> indicies = { -1, -1, -1, -1, -1, -1, -1 };
			std::vector<VFX*> temp;

			int index = 0;
			for (VFX* effect : m_effects) {
				
				int i = effect->getType();
				indicies[i] = index; // this implies that there is only one type of each effect
				index++;
			}

			for (const auto& position : indicies) {
				if (position > -1)
					temp.push_back(m_effects.at(position));

			}

			m_effects.swap(temp);

		}


		void renderPipeline()  {
			//assert(m_context.isTargetReady);
			static bool once = false;
			if (!once) {
				Renderer::Texture::writeToFile(m_context.targetTexture, "inrenderpipeline.png");
				once = true;
			}
			for (VFX* effect : m_effects) {

				if (!effect->isEnabled()) continue;

				m_context.targetTexture = std::move(effect->applyEffect(m_context.targetTexture)); // TODO change to a context parameter

				std::stringstream ss;
				ss << effect->getName();
				ss << ".png";


				Renderer::Texture::writeToFile(m_context.targetTexture, ss.str().c_str());

				ss.str(std::string());
			}



		}

		void onImGuiRender() const {
			for (VFX* effect : m_effects)
				effect->onImGuiRender();
		}

		const Renderer::Texture &getTargetTexture() const {
			return m_context.targetTexture;
		}

		//-------- Helpers

		void printCurrentEffectOrder() const {

			for (VFX* effect : m_effects)
				std::cout << effect->getName() << std::endl;
		}
	};


}