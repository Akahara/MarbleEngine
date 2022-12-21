#pragma once

#include "../Scene.h"

#include "../../abstraction/Camera.h"
#include "../../abstraction/pipeline/VFXPipeline.h"
#include "../../abstraction/pipeline/Bloom.h"


#include "../../abstraction/pipeline/Saturation.h"
#include "../../abstraction/pipeline/GammaCorrection.h"
#include "../../abstraction/pipeline/LensMask.h"
#include "../../abstraction/pipeline/Contrast.h"
#include "../../abstraction/pipeline/Flares.h"


#include "TestTerrain.h"



class TestFBScene : public Scene {
private:
  Scene                      *m_backingScene;
  visualEffects::VFXPipeline  m_pipeline{ Window::getWinWidth(), Window::getWinHeight() };
  Renderer::BlitPass          m_blitData;
  BloomRenderer               m_renderer;
  Renderer::FrameBufferObject m_fbo;
  Renderer::Texture           target{ Window::getWinWidth(), Window::getWinHeight() };
  
  glm::vec3                   m_sun{100,100,100};
  Renderer::Mesh              m_cube = Renderer::createCubeMesh();

public:
  TestFBScene()
  {
    m_backingScene = new TestTerrainScene;

    m_pipeline.registerEffect<visualEffects::LensMask>();
    
    m_pipeline.addContextParam<glm::vec3>({ 10,10,10 }, "sunPos");
    m_pipeline.addContextParam<glm::vec3>({ 10,10,10 }, "cameraPos");
    m_pipeline.addContextParam<Renderer::Camera>(m_backingScene->getCamera(), "camera");
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
      m_pipeline.setContextParam<glm::vec3>("cameraPos", m_backingScene->getCamera().getForward());
      m_pipeline.setContextParam<Renderer::Camera>("camera", m_backingScene->getCamera());
      m_pipeline.bind();
      m_backingScene->onRender();
      Renderer::renderMesh(m_backingScene->getCamera(), m_sun, { 5,5,5 }, m_cube);
      m_pipeline.unbind();
      
      m_pipeline.renderPipeline();
  }

  void onImGuiRender() override 
  {
    m_backingScene->onImGuiRender();
    glm::vec3 campos = m_backingScene->getCamera().getPosition();
    
    m_pipeline.onImGuiRender();

    ImGui::DragFloat3("sunPos", &m_sun.x, 1.F);
    ImGui::Text("Player position : {%f, %f, %f}", campos.x, campos.y, campos.z );
    ImGui::Text("Dot product : %f", glm::dot(glm::normalize(m_sun), glm::normalize(m_backingScene->getCamera().getForward())));
  }

  CAMERA_NOT_DEFINED();
};