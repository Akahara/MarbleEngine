#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"

class Test2DScene : public Scene {
private:
  Renderer::Camera m_camera;
  float            m_time = 0;
public:
  Test2DScene()
  {
    m_camera.setProjection(Renderer::OrthographicProjection{ -1.0f, 1.0f, -1.0f, 1.0f });
  }

  void step(float delta) override
  {
    m_time += delta;
  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    Renderer::Renderer::beginBatch(m_camera);
    for (float y = -1.0f; y < 1.0f; y += 0.025f) {
      for (float x = -1.0f; x < 1.0f; x += 0.025f) {
        glm::vec4 color = { (x + 1.0f) / 2.0f * (sin(m_time * 2) + 1),
                            (x + y) / 2.0f * (cos(m_time * 7) + 1),
                            (y + 1.0f) / 2.0f * (sin(m_time * 2) + 1),
                            1.0f };
        Renderer::Renderer::drawQuad({ x,y, 0.0f }, { 0.02f, 0.02f }, color);
      }
    }
    Renderer::Renderer::endBatch();
    Renderer::Renderer::flush();
  }

  void onImGuiRender() override {}
};