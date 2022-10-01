#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/Player.h"

class Test3DScene : public Scene {
private:
  Renderer::Cubemap m_skybox;
  Player            m_player;
  Renderer::Mesh    m_cubeMesh;
public:
  Test3DScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
      m_cubeMesh{ Renderer::CreateCubeMesh() }
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
    //Renderer::RenderCube({ 1, 0, 0 }, glm::vec3{ 2.f, .05f, .05f }, { 0.f, 0.f, 1.f }, m_player.GetCamera().getViewProjectionMatrix()); // +x blue
    //Renderer::RenderCube({ 0, 1, 0 }, glm::vec3{ .05f, 2.f, .05f }, { 1.f, 0.f, 0.f }, m_player.GetCamera().getViewProjectionMatrix()); // +y red
    //Renderer::RenderCube({ 0, 0, 1 }, glm::vec3{ .05f, .05f, 2.f }, { 0.f, 1.f, 0.f }, m_player.GetCamera().getViewProjectionMatrix()); // +z green
    //Renderer::RenderCube({ 0, 0, 0 }, glm::vec3{ 1.f,  1.f,  1.f }, { .9f, .9f, .9f }, m_player.GetCamera().getViewProjectionMatrix()); // unit cube
    Renderer::RenderMesh({ 1, 0, 0 }, { 0.f, 0.f, 1.f }, m_cubeMesh, m_player.GetCamera().getViewProjectionMatrix()); // +x blue
    Renderer::RenderMesh({ 0, 1, 0 }, { 1.f, 0.f, 0.f }, m_cubeMesh, m_player.GetCamera().getViewProjectionMatrix()); // +y red
    Renderer::RenderMesh({ 0, 0, 1 }, { 0.f, 1.f, 0.f }, m_cubeMesh, m_player.GetCamera().getViewProjectionMatrix()); // +z green
    Renderer::RenderMesh({ 0, 0, 0 }, { .9f, .9f, .9f }, m_cubeMesh, m_player.GetCamera().getViewProjectionMatrix()); // unit cube
  }

  void OnImGuiRender() override {}
};