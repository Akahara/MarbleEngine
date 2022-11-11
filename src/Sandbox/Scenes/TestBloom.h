#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"

#include "../../abstraction/pipeline/Bloom.h"
#include "../../world/Light/Light.h"


class TestBloomScene : public Scene {
private:


    Renderer::Cubemap  m_skybox;
    Player             m_player;
    Renderer::Mesh m_cubeMesh = Renderer::createCubeMesh();

    std::vector<Light> m_lights;
    Light m_testLight;

    Light::LightParam m_params { 
        {0.5,0.5,0},
        {0,0.5,0},
        {0.5,0,0.5}
    };

    float m_distance;
    glm::vec3 m_lightpos{ 10.f };
    bool m_turnOn = false;
    bool m_lightsOn[12] =
    {
        0,0,0,
        0,0,0,
        0,0,0,
        0,0,0
    };

public:
    TestBloomScene() : m_skybox {
        "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
            "res/skybox_dbg/skybox_left.bmp", "res/skybox_dbg/skybox_right.bmp",
            "res/skybox_dbg/skybox_top.bmp", "res/skybox_dbg/skybox_bottom.bmp"
    }
    {
       
        Renderer::setUniformPointLights(m_lights);
    }

    ~TestBloomScene()
    {
    }

    void step(float delta) override
    {
        m_player.step(delta);
    }

    void onRender() override
    {

        Renderer::clear();

        Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());

        for (const auto& light : m_lights) {

            Renderer::renderMesh(light.getPosition(), glm::vec3(3), m_cubeMesh, m_player.getCamera());
        }

        Renderer::renderMesh({0,0,0}, glm::vec3(3), m_cubeMesh, m_player.getCamera());
        Renderer::renderMesh({10,0,0}, glm::vec3(3), m_cubeMesh, m_player.getCamera());
        Renderer::renderMesh({0,10,0}, glm::vec3(3), m_cubeMesh, m_player.getCamera());
        Renderer::renderMesh({0,0,10}, glm::vec3(3), m_cubeMesh, m_player.getCamera());

    }

    void onImGuiRender() override
    {

        if (m_lights.size() < 12) {

            if (ImGui::Button("Generate a light")) 
            {
                m_lights.push_back({});
            }

        }

        for (unsigned int i = 0; i < m_lights.size(); i++) {

            Light& light = m_lights.at(i);

            if (ImGui::Checkbox((std::stringstream{ "Switch n" } << i).str().c_str(), &m_lightsOn[i])) {

                m_lights.at(i).setOn(m_lightsOn[i]);
                Renderer::setUniformPointLights(m_lights);
            }

            if (m_lights.at(i).isOn()) {



                glm::vec3 pos = light.getPosition();
                Light::LightParam params = light.getParams();
                float distance = light.getDistance();

                std::stringstream ss{ std::string() };
                ss << "Light " << i+1;

                if (ImGui::CollapsingHeader(ss.str().c_str())) {

                    if (
                        ImGui::DragFloat3((std::stringstream{ "LightPosition n" } << i).str().c_str(), &pos.x, 2.f) +
                        ImGui::SliderFloat3((std::stringstream{ "Ambiant n" } << i).str().c_str(), &params.ambiant.x, 0, 1) +
                        ImGui::SliderFloat3((std::stringstream{"Diffuse n" } << i).str().c_str(), &params.diffuse.x, 0, 1) +
                        ImGui::SliderFloat3((std::stringstream{"Specular n" } << i).str().c_str(), &params.specular.x, 0, 1) +

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

    }
};