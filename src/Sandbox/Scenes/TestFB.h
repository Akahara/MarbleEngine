#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"

#include "Test2D.h"
#include "Test3D.h"
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
    m_fbDepthTexture = Renderer::Texture::createDepthTexture(m_fbRenderTexture.GetWidth(), m_fbRenderTexture.GetHeight());
    m_fb.SetTargetTexture(m_fbRenderTexture);
    m_fb.SetDepthTexture(m_fbDepthTexture);
  }

  ~TestFBScene()
  {
    delete m_backingScene;
  }

  void Step(float delta) override
  {
    m_backingScene->Step(delta);
  }

  void OnRender() override
  {
    m_fb.Bind();
    Renderer::FrameBufferObject::SetViewportToTexture(m_fbRenderTexture);

    m_backingScene->OnRender();

    m_fb.Unbind();
    Renderer::FrameBufferObject::SetViewport(Window::getWinWidth(), Window::getWinHeight());

    m_blitData.DoBlit(m_fbRenderTexture);
  }

  void OnImGuiRender() override 
  {
    m_backingScene->OnImGuiRender();

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
          m_blitData.GetShader().Bind();
          m_blitData.GetShader().SetUniform1f("u_vignetteMin", vignetteMin);
          m_blitData.GetShader().SetUniform1f("u_vignetteMax", vignetteMax);
          m_blitData.GetShader().SetUniform1f("u_vignetteStrength", vignetteStrength * vignetteEnable);
        }
      }
    }
    ImGui::End();
  }
};