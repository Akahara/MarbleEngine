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
    Renderer::Mesh     m_cubeMesh{ Renderer::createCubeModel(), std::make_shared<Renderer::Material>() };
    World::Sky         m_sky;

    visualEffects::VFXPipeline m_pipeline{ Window::getWinWidth(), Window::getWinHeight() };
    World::LightRenderer       m_lRenderer;

    float m_exposure = 1.0f;
    float m_strenght = 0.05f;

    float m_realtime = 0;
    

public:
    TestBloomScene()
    {
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

        m_cubeMesh.getMaterial()->shader = Renderer::getStandardMeshShader();
        m_cubeMesh.getTransform().scale = { 3,3,3 };
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

        // in a real scene an InstancedMesh would be used
        m_cubeMesh.getTransform().position = { 0,0,0  }; Renderer::renderMesh(camera, m_cubeMesh);
        m_cubeMesh.getTransform().position = { 10,0,0 }; Renderer::renderMesh(camera, m_cubeMesh);
        m_cubeMesh.getTransform().position = { 0,10,0 }; Renderer::renderMesh(camera, m_cubeMesh);
        m_cubeMesh.getTransform().position = { 0,0,10 }; Renderer::renderMesh(camera, m_cubeMesh);

        m_sky.render(camera, m_realtime);

        m_pipeline.unbind();
        m_pipeline.renderPipeline();
    }

    void onImGuiRender() override
    {
        m_pipeline.onImGuiRender();
        m_lRenderer.onImguiRender();
    }

    CAMERA_IS_PLAYER(m_player);
};