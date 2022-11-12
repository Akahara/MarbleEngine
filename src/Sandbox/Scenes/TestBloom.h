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
    Renderer::Mesh m_cubeMesh = Renderer::createCubeMesh(1);
    Renderer::FrameBufferObject m_fbo;
    Renderer::Texture m_target{ Window::getWinWidth(), Window::getWinHeight() };
    Renderer::Texture m_depth = Renderer::Texture::createDepthTexture(Window::getWinWidth(), Window::getWinHeight());
    bool write = false;



    std::vector<Light> m_lights;
    Renderer::BlitPass m_blit;

    BloomRenderer m_bloomRenderer;

    float m_exposure = 1.0f;

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
        m_fbo.setTargetTexture(m_target);
        m_fbo.setDepthTexture(m_depth);
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
        Renderer::FrameBufferObject::setViewportToWindow();
        
        m_fbo.bind();

        Renderer::clear();

        Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());

        for (const auto& light : m_lights) {

            Renderer::renderMesh(light.getPosition(), glm::vec3(3), m_cubeMesh, m_player.getCamera());
        }

        Renderer::renderMesh({0,0,0}, glm::vec3(3), m_cubeMesh, m_player.getCamera());
        Renderer::renderMesh({10,0,0}, glm::vec3(3), m_cubeMesh, m_player.getCamera());
        Renderer::renderMesh({0,10,0}, glm::vec3(3), m_cubeMesh, m_player.getCamera());
        Renderer::renderMesh({0,0,10}, glm::vec3(3), m_cubeMesh, m_player.getCamera());

        m_fbo.unbind();
        m_blit.doBlit(m_target);
        



        Renderer::Shader::unbind();

        m_bloomRenderer.RenderBloomTexture(m_target, 0.005f, write);
        if (write) write = false;

        m_blit.doBlit(m_target);


    }

    void onImGuiRender() override
    {
        if (ImGui::SliderFloat("Exposure", &m_exposure, 0, 5)) {
            m_blit.getShader().bind();
            m_blit.getShader().setUniform1f("u_exposure", m_exposure);
        }

        

        if (ImGui::Button("writeMips")) {

            Renderer::Texture::writeToFile(m_target, "og_prev.png");
            write = true;
        }

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
                        ImGui::SliderFloat3((std::stringstream{ "Ambiant n" } << i).str().c_str(), &params.ambiant.x, 0, 5) +
                        ImGui::SliderFloat3((std::stringstream{"Diffuse n" } << i).str().c_str(), &params.diffuse.x, 0, 5) +
                        ImGui::SliderFloat3((std::stringstream{"Specular n" } << i).str().c_str(), &params.specular.x, 0, 5) +

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