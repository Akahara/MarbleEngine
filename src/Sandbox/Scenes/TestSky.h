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
    TempRenderer::RenderCube({ 1, 0, 0 }, glm::vec3{ 2.f, .05f, .05f }, { 0.f, 0.f, 1.f }, m_player.GetCamera().getViewProjectionMatrix()); // +x blue
    TempRenderer::RenderCube({ 0, 1, 0 }, glm::vec3{ .05f, 2.f, .05f }, { 1.f, 0.f, 0.f }, m_player.GetCamera().getViewProjectionMatrix()); // +y red
    TempRenderer::RenderCube({ 0, 0, 1 }, glm::vec3{ .05f, .05f, 2.f }, { 0.f, 1.f, 0.f }, m_player.GetCamera().getViewProjectionMatrix()); // +z green
    TempRenderer::RenderCube({ 0, 0, 0 }, glm::vec3{ 1.f,  1.f,  1.f }, { .9f, .9f, .9f }, m_player.GetCamera().getViewProjectionMatrix()); // unit cube
  }

  void OnImGuiRender() override {}
};