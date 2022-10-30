#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"
#include "../../abstraction/pipeline/VFXPipeline.h"
#include "../../abstraction/pipeline/Saturation.h"

#include "Test2D.h"
#include "TestTerrain.h"



class TestFBScene : public Scene {
private:
  Scene                      *m_backingScene;
  visualEffects::VFXPipeline  m_pipeline{ Window::getWinWidth(), Window::getWinHeight() };
  Renderer::BlitPass            m_blitData;

  public:
  TestFBScene()
  {
    m_backingScene = new TestTerrainScene;
    m_pipeline.registerEffect<visualEffects::Saturation>();

    m_pipeline.setShaderOfEffect(visualEffects::SaturationEffect, "res/shaders/saturation.fs");

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
      m_pipeline.bind();

      m_backingScene->onRender();

      m_pipeline.unbind();


      m_pipeline.renderPipeline();

  }

  void onImGuiRender() override 
  {
    m_backingScene->onImGuiRender();
    m_pipeline.onImGuiRender();

    ImGui::End();
  }
};