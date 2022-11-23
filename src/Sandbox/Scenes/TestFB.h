#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"
#include "../../abstraction/pipeline/VFXPipeline.h"
#include "../../abstraction/pipeline/Bloom.h"


#include "../../abstraction/pipeline/Saturation.h"
#include "../../abstraction/pipeline/GammaCorrection.h"
#include "../../abstraction/pipeline/LensMask.h"
#include "../../abstraction/pipeline/Contrast.h"


#include "Test2D.h"
#include "TestTerrain.h"



class TestFBScene : public Scene {
private:
  Scene                      *m_backingScene;
  visualEffects::VFXPipeline  m_pipeline{ Window::getWinWidth(), Window::getWinHeight() };
  Renderer::BlitPass            m_blitData;
  BloomRenderer m_renderer;
  Renderer::FrameBufferObject m_fbo;
  Renderer::Texture target{ Window::getWinWidth(), Window::getWinHeight() };
  
  glm::vec3 m_sun{10,10,10};
  glm::vec3 m_camera{10,10,10};


  public:
  TestFBScene()
  {
    m_backingScene = new TestTerrainScene;


    m_pipeline.registerEffect<visualEffects::Saturation>();
    m_pipeline.registerEffect<visualEffects::GammaCorrection>();
    m_pipeline.registerEffect<visualEffects::Contrast>();
    m_pipeline.registerEffect<visualEffects::Sharpness>();
    m_pipeline.registerEffect<visualEffects::LensMask>();
    
    m_pipeline.setShaderOfEffect(visualEffects::SaturationEffect,       "res/shaders/saturation.fs"     );
    m_pipeline.setShaderOfEffect(visualEffects::GammaCorrectionEffect,  "res/shaders/gammacorrection.fs");
    m_pipeline.setShaderOfEffect(visualEffects::ContrastEffect,         "res/shaders/contrast.fs"       );
    m_pipeline.setShaderOfEffect(visualEffects::SharpnessEffect,         "res/shaders/sharpness.fs"       );

//    m_pipeline.sortPipeline();
    
    m_pipeline.addContextParam<glm::vec3>({ 10,10,10 }, "sunPos");
    m_pipeline.addContextParam<glm::vec3>({ 10,10,10 }, "cameraPos");
    m_pipeline.addContextParam<Renderer::Camera>(m_backingScene->camera, "camera");
    m_fbo.setTargetTexture(target);
    m_fbo.setViewportToTexture(target);

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
      m_pipeline.setContextParam<glm::vec3>("sunPos", m_sun);
      m_pipeline.setContextParam<glm::vec3>("cameraPos", m_backingScene->camera.getForward());
      m_pipeline.setContextParam<Renderer::Camera>("camera", m_backingScene->camera);
      m_pipeline.bind();
      m_backingScene->onRender();
      m_pipeline.unbind();
      
      m_pipeline.renderPipeline();

      /*
      m_fbo.bind();
      m_backingScene->onRender();
      m_fbo.unbind();

      Renderer::clear();
      m_backingScene->onRender();
      Renderer::renderDebugGUIQuadWithTexture(target, { 0.5, 0.5 }, { 0.5,0.5 });
      */

  }

  void onImGuiRender() override 
  {
    m_backingScene->onImGuiRender();
    
    m_pipeline.onImGuiRender();


    ImGui::DragFloat3("sunPos", &m_sun.x, 1.F);
    ImGui::DragFloat3("cameraPos", &m_camera.x, 1.F);
    std::cout << glm::dot(glm::normalize(m_sun), glm::normalize(m_backingScene->camera.getForward())) << std::endl;
  }
};