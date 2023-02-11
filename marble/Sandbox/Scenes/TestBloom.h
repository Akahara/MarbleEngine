#pragma once

#include "../Scene.h"

#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Camera.h"
#include "../../abstraction/pipeline/Bloom.h"
#include "../../abstraction/pipeline/VFXPipeline.h"
#include "../../World/Player.h"
#include "../../World/Sky.h"
#include "../../World/Light/Light.h"
#include "../../World/Light/LightManager.h"

class TestBloomScene : public Scene {
private:
    Player             m_player;
    Renderer::Mesh     m_cubeMesh = Renderer::createCubeMesh();
    Renderer::FrameBufferObject m_fbo;
    Renderer::Texture  m_target{ Window::getWinWidth(), Window::getWinHeight() };
    Renderer::Texture  m_depth = Renderer::Texture::createDepthTexture(Window::getWinWidth(), Window::getWinHeight());

    World::Sky         m_sky;

    visualEffects::VFXPipeline  m_pipeline{ Window::getWinWidth(), Window::getWinHeight() };

    std::vector<Light> m_lights;
    Renderer::BlitPass m_blit;
    Renderer::BlitPass m_blitFinal;

    World::LightRenderer m_lRenderer;

    BloomRenderer      m_bloomRenderer;

    float m_exposure = 1.0f;
    float m_strenght = 0.05f;

    float m_realtime = 0;
    /*
    bool m_lightsOn[12] =
    {
        0,0,0,
        0,0,0,
        0,0,0,
        0,0,0
    };
    */

public:
    TestBloomScene()
    {
        m_fbo.setTargetTexture(m_target);
        m_fbo.setDepthTexture(m_depth);
        Renderer::setUniformPointLights(m_lights);
        m_blitFinal.setShader("res/shaders/bloom/finalBloom.fs");
        m_blitFinal.getShader().bind();
        m_blitFinal.getShader().setUniform1i("u_sceneTexture", 0);
        m_blitFinal.getShader().setUniform1i("u_finalBloom", 1);
        m_blitFinal.getShader().unbind();

        //===============//
        m_pipeline.registerEffect<visualEffects::Saturation>();
        m_pipeline.registerEffect<visualEffects::GammaCorrection>();
        m_pipeline.registerEffect<visualEffects::Contrast>();
        m_pipeline.registerEffect<visualEffects::Sharpness>();

        m_pipeline.setShaderOfEffect(visualEffects::SaturationEffect,       "res/shaders/saturation.fs");
        m_pipeline.setShaderOfEffect(visualEffects::GammaCorrectionEffect,  "res/shaders/gammacorrection.fs");
        m_pipeline.setShaderOfEffect(visualEffects::ContrastEffect,         "res/shaders/contrast.fs");
        m_pipeline.setShaderOfEffect(visualEffects::SharpnessEffect,        "res/shaders/sharpness.fs");

        m_pipeline.registerEffect<visualEffects::Bloom>();
        m_pipeline.sortPipeline();
    }

    ~TestBloomScene()
    {
    }

    void step(float delta) override
    {
        m_player.step(delta);
        m_realtime += delta;
    }

    void onRender() override
    {
        const Renderer::Camera& camera = m_player.getCamera();
        Renderer::FrameBufferObject::setViewportToWindow();
        
        m_pipeline.bind();

        Renderer::clear();
        Renderer::Texture::unbind(0);

        for (const auto& light : m_lights) {
            Renderer::renderMesh(camera, light.getPosition(), glm::vec3(3), m_cubeMesh);
        }

        Renderer::renderMesh(camera, {0,0,0}, glm::vec3(3), m_cubeMesh);
        Renderer::renderMesh(camera, {10,0,0}, glm::vec3(3), m_cubeMesh);
        Renderer::renderMesh(camera, {0,10,0}, glm::vec3(3), m_cubeMesh);
        Renderer::renderMesh(camera, {0,0,10}, glm::vec3(3), m_cubeMesh);

        m_sky.render(camera, m_realtime);

        m_pipeline.unbind();

        m_pipeline.renderPipeline();
    }

    void onImGuiRender() override
    {
        m_pipeline.onImGuiRender();

        if (ImGui::SliderFloat("bloomStrength", &m_strenght, 0, 5)) {
            m_blitFinal.getShader().bind();
            m_blitFinal.getShader().setUniform1f("u_bloomStrength", m_strenght);
            m_blitFinal.getShader().unbind();
        }

        m_lRenderer.onImguiRender();
       
        
    }

    CAMERA_IS_PLAYER(m_player);
};