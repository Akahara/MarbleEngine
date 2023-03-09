#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "../../abstraction/UnifiedRenderer.h"
#include "../../vendor/imgui/imgui.h"




// TODO : This class has to change alot
// rename, and change the ligth class

namespace World {

	class LightRenderer {

	private:
		std::vector<Light> m_lights;

		bool m_lightsOn[12] =
		{
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0
		};

		int m_lightDistanceIndex[12] =
		{
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0
		};


	public:

		void uploadLightsToShader(Renderer::Shader& shader) {

			shader.bind();

			shader.setUniform1i("u_numberOfLights", (int)m_lights.size());

			for (int i = 0; i < m_lights.size(); i++) {
				const Light& light = m_lights.at(i);

				std::stringstream ss{ std::string() };
				ss << "u_lights[";
				ss << i;
				ss << "].";
				std::string lightInShader = ss.str();
				shader.setUniform1i(lightInShader + "on", light.isOn());
				shader.setUniform3f(lightInShader + "position", light.getPosition());
				shader.setUniform1f(lightInShader + "constant", light.getCoefs().constant);
				shader.setUniform1f(lightInShader + "linear", light.getCoefs().linear);
				shader.setUniform1f(lightInShader + "quadratic", light.getCoefs().quadratic);
				shader.setUniform3f(lightInShader + "ambient", light.getParams().ambiant);
				shader.setUniform3f(lightInShader + "diffuse", light.getParams().diffuse);
				shader.setUniform3f(lightInShader + "specular", light.getParams().specular);
			}

			shader.unbind();
		}


		void onImguiRender() {
			
			ImGui::Begin("Lights controls");
			{ 
				if (ImGui::Button("Generate a light") && m_lights.size() < 12)
					m_lights.push_back(Light{
						});

				for (unsigned int i = 0; i < m_lights.size(); i++) {
					Light& light = m_lights.at(i);

					std::stringstream ss{ std::string() };
					ss << "Light " << i + 1;

					if (ImGui::Checkbox(ss.str().c_str(), &m_lightsOn[i])) {
						m_lights.at(i).setOn(m_lightsOn[i]);
						Renderer::setUniformPointLights(m_lights);
					}

					if (!m_lights.at(i).isOn())
						continue;

					glm::vec3 pos = light.getPosition();
					Light::LightParam params = light.getParams();
					float distance = light.getDistance();

					if (!ImGui::CollapsingHeader(ss.str().c_str()))
						continue;


					if (ImGui::DragFloat3("Position ##" + i, &pos.x, 2.f) +
						ImGui::ColorEdit3("Ambiant ##" + i, &params.ambiant.x) +
						ImGui::ColorEdit3("Diffuse ##" + i, &params.diffuse.x) +
						ImGui::ColorEdit3("Specular ##" + i, &params.specular.x) +
						ImGui::SliderInt("Distance ##" + i, &m_lightDistanceIndex[i], 0, 11)) {

						Light l = Light{
						  pos,
						  params,
						  s_keys[m_lightDistanceIndex[i]],
						  m_lightsOn[i]
						};

						std::cout << l.getDistance() << std::endl;
						m_lights.at(i) = l;

						Renderer::setUniformPointLights(m_lights);
					}
				}
			}

			ImGui::End();
		}
	};
}