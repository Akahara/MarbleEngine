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
  void step(float delta) override
  {
    m_player.step(delta);
    m_sky.step(delta);
  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    m_sky.render(m_player);
  }

  void onImGuiRender() override {}
};