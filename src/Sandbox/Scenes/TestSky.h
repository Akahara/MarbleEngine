#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../World/Player.h"
#include "../../World/Sky.h"

class TestSkyScene : public Scene {
private:
  Player     m_player;
  World::Sky m_sky;
public:
  void Step(float delta) override
  {
    m_player.Step(delta);
    m_sky.Step(delta);
  }

  void OnRender() override
  {
    Renderer::Renderer::Clear();
    m_sky.Render(m_player);
  }

  void OnImGuiRender() override {}
};