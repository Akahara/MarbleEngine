#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/Player.h"

class TestShadowsScene : public Scene {
private:
  Renderer::Cubemap m_skybox;
  Player            m_player;
  Renderer::Mesh    m_mesh1;
public:
  TestShadowsScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
      m_mesh1(Renderer::LoadMeshFromFile("res/models/floor.obj"))
  {
  }

  void Step(float delta) override
  {
    m_player.Step(delta);
  }

  void OnRender() override
  {
    Renderer::Renderer::Clear();
    Renderer::CubemapRenderer::DrawCubemap(m_skybox, m_player.GetCamera(), m_player.GetPosition());
    Renderer::RenderMesh({ 0, 0, 0 }, { 1.f, 1.f, 1.f }, m_mesh1, m_player.GetCamera().getViewProjectionMatrix());
  }

  void OnImGuiRender() override {}
};