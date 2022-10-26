#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"

#include "Test2D.h"
#include "TestTerrain.h"



class TestFBScene : public Scene {
private:
  Scene                      *m_backingScene;
  Renderer::FrameBufferObject m_fb;
  Renderer::Texture           m_fbRenderTexture;
  Renderer::Texture           m_fbDepthTexture;
  Renderer::BlitPass          m_blitData;
public:
  TestFBScene()
  {
    m_backingScene = new TestTerrainScene;
    m_fbRenderTexture = Renderer::Texture(16*5, 9*5);
    m_fbDepthTexture = Renderer::Texture::createDepthTexture(m_fbRenderTexture.getWidth(), m_fbRenderTexture.getHeight());
    m_fb.setTargetTexture(m_fbRenderTexture);
    m_fb.setDepthTexture(m_fbDepthTexture);
  }

  ~TestFBScene()
  {
    delete m_backingScene;
  }

  void step(float delta) override
  {
    m_backingScene->step(delta);
  }

  void onRender() override
  {
    m_fb.bind();
    Renderer::FrameBufferObject::setViewportToTexture(m_fbRenderTexture);

    m_backingScene->onRender();

    Renderer::FrameBufferObject::unbind();
    Renderer::FrameBufferObject::setViewport(Window::getWinWidth(), Window::getWinHeight());

    m_blitData.doBlit(m_fbRenderTexture);
  }

  void onImGuiRender() override 
  {
    m_backingScene->onImGuiRender();

    if (ImGui::Begin("VFX")) {
      if (ImGui::CollapsingHeader("Vignette")) {
        static float vignetteMin = 1.2f;
        static float vignetteMax = .045f;
        static float vignetteStrength = .2f;
        static bool vignetteEnable = true;
        if (ImGui::Checkbox("vignette", &vignetteEnable) +
            ImGui::SliderFloat("min", &vignetteMin, 0.f, 2.f) +
            ImGui::SliderFloat("max", &vignetteMax, 0.f, 2.f) +
            ImGui::SliderFloat("strength", &vignetteStrength, -1.f, 1.f)) {
          m_blitData.getShader().bind();
          m_blitData.getShader().setUniform1f("u_vignetteMin", vignetteMin);
          m_blitData.getShader().setUniform1f("u_vignetteMax", vignetteMax);
          m_blitData.getShader().setUniform1f("u_vignetteStrength", vignetteStrength * vignetteEnable);
        }
      }
    }
    ImGui::End();
  }
};