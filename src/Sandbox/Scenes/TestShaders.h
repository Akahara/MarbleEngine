#pragma once

#include "../Scene.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Camera.h"
#include "../../World/Player.h"

class TestShadersScene : public Scene {
private:
  Player                  m_player;
  Renderer::Mesh          m_planeMesh;
  Renderer::Shader        m_shader;
  Renderer::ShaderManager m_shaderManager;
public:
  TestShadersScene()
  {
    m_player.setPostion({ 0, 2, 0 });
    m_player.setRotation(0, +3.14f*.5f);
    m_player.updateCamera();
    m_shaderManager.addShader(&m_shader, "res/shaders/test_scene.vs", "res/shaders/test_scene.fs");
    m_planeMesh = Renderer::createPlaneMesh();
  }

  void step(float delta) override
  {
    m_player.step(delta);
  }

  void onRender() override
  {
    Renderer::clear();

    glm::mat4 M(1.f);
    //M = glm::translate(M, { 0.f, 0.f, 0.f });
    //M = glm::scale(M, { 1.f, 1.f, 1.f });
    m_shader.bind();
    m_shader.setUniformMat4f("u_M", M);
    m_shader.setUniformMat4f("u_VP", m_player.getCamera().getViewProjectionMatrix());
    m_planeMesh.draw();
  }

  void onImGuiRender() override
  {
    if (ImGui::Begin("Scene")) {
      m_shaderManager.promptReloadAndUI();
    }
    ImGui::End();
  }
};