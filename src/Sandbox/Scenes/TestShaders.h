#pragma once

#include "../Scene.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"
#include "../../World/Player.h"

class TestShadersScene : public Scene {
private:
  Player           m_player;
  Renderer::Mesh   m_planeMesh;
  Renderer::Shader m_shader;
  Renderer::ShaderManager m_shaderManager;
public:
  TestShadersScene()
  {
    m_player.SetPostion({ 0, 2, 0 });
    m_player.SetRotation(0, +3.14f*.5f);
    m_player.UpdateCamera();
    m_shaderManager.AddShader(&m_shader, "res/shaders/test_scene.vs", "res/shaders/test_scene.fs");
    m_planeMesh = Renderer::CreatePlaneMesh();
  }

  void Step(float delta) override
  {
    m_player.Step(delta);
  }

  void OnRender() override
  {
    Renderer::Renderer::Clear();

    glm::mat4 M(1.f);
    //M = glm::translate(M, { 0.f, 0.f, 0.f });
    //M = glm::scale(M, { 1.f, 1.f, 1.f });
    m_shader.Bind();
    m_shader.SetUniformMat4f("u_M", M);
    m_shader.SetUniformMat4f("u_VP", m_player.GetCamera().getViewProjectionMatrix());
    m_planeMesh.Draw();
  }

  void OnImGuiRender() override
  {
    if (ImGui::Begin("Scene")) {
      m_shaderManager.PromptReload();
    }
    ImGui::End();
  }
};