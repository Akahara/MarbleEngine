#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "../../abstraction/UnifiedRenderer.h"
#include "../../vendor/imgui/imgui.h"

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
			Renderer::Shader& meshShader = Renderer::getStandardMeshShader();
			meshShader.bind();
			Renderer::setUniformPointLights(m_lights);
			meshShader.unbind();
		}


		void onImguiRender() {
			// light controls
			{ 
				if (ImGui::Button("Generate a light") && m_lights.size() < 12)
					m_lights.push_back({});

				for (unsigned int i = 0; i < m_lights.size(); i++) {
					Light& light = m_lights.at(i);

					if (ImGui::Checkbox((std::stringstream{ "Switch n" } << i).str().c_str(), &m_lightsOn[i])) {
						m_lights.at(i).setOn(m_lightsOn[i]);
						Renderer::setUniformPointLights(m_lights);
					}

					if (!m_lights.at(i).isOn())
						continue;

					glm::vec3 pos = light.getPosition();
					Light::LightParam params = light.getParams();
					float distance = light.getDistance();

					std::stringstream ss{ std::string() };
					ss << "Light " << i + 1;

					if (!ImGui::CollapsingHeader(ss.str().c_str()))
						continue;

					if (ImGui::DragFloat3("Position ##" + i, &pos.x, 2.f) +
						ImGui::SliderFloat3("Ambiant ##" + i, &params.ambiant.x, 0, 15) +
						ImGui::SliderFloat3("Diffuse ##" + i, &params.diffuse.x, 0, 15) +
						ImGui::SliderFloat3("Specular ##" + i, &params.specular.x, 0, 15) +
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
		}
	};
}