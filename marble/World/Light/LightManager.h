#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "../../abstraction/UnifiedRenderer.h"
#include "../../vendor/imgui/imgui.h"


// TODO : This class has to change alot


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
		LightRenderer() {
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