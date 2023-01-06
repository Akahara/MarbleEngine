#pragma once


#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "../../abstraction/UnifiedRenderer.h"
#include "../../vendor/imgui/imgui.h"

namespace World {

	class LightRenderer {

	private:
		std::vector<Light> m_lights;
		//std::vector<bool> m_isOn;

		bool m_lightsOn[12] =
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

					if (ImGui::DragFloat3((std::stringstream{ "LightPosition n" } << i).str().c_str(), &pos.x, 2.f) +
						ImGui::SliderFloat3((std::stringstream{ "Ambiant n" } << i).str().c_str(), &params.ambiant.x, 0, 15) +
						ImGui::SliderFloat3((std::stringstream{ "Diffuse n" } << i).str().c_str(), &params.diffuse.x, 0, 15) +
						ImGui::SliderFloat3((std::stringstream{ "Specular n" } << i).str().c_str(), &params.specular.x, 0, 15) +
						ImGui::DragFloat((std::stringstream{ "Distance n" } << i).str().c_str(), &distance, 30.f)) {

						Light l = Light{
						  pos,
						  params,
						  distance,
						  m_lightsOn[i]
						};
						m_lights.at(i) = l;

						Renderer::setUniformPointLights(m_lights);
					}
				}
			}
		}
	};
}