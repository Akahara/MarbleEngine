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
      m_cubeMesh{ Renderer::createCubeMesh() }
  {
  }

  void step(float delta) override
  {
    m_player.step(delta);
  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera(), m_player.getPosition());
    //Renderer::RenderCube({ 1, 0, 0 }, glm::vec3{ 2.f, .05f, .05f }, { 0.f, 0.f, 1.f }, m_player.getCamera().getViewProjectionMatrix()); // +x blue
    //Renderer::RenderCube({ 0, 1, 0 }, glm::vec3{ .05f, 2.f, .05f }, { 1.f, 0.f, 0.f }, m_player.getCamera().getViewProjectionMatrix()); // +y red
    //Renderer::RenderCube({ 0, 0, 1 }, glm::vec3{ .05f, .05f, 2.f }, { 0.f, 1.f, 0.f }, m_player.getCamera().getViewProjectionMatrix()); // +z green
    //Renderer::RenderCube({ 0, 0, 0 }, glm::vec3{ 1.f,  1.f,  1.f }, { .9f, .9f, .9f }, m_player.getCamera().getViewProjectionMatrix()); // unit cube
    Renderer::renderMesh({ 1, 0, 0 }, { .1f, .1f, 1.f }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix()); // +x blue
    Renderer::renderMesh({ 0, 1, 0 }, { 1.f, .1f, .1f }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix()); // +y red
    Renderer::renderMesh({ 0, 0, 1 }, { .1f, 1.f, .1f }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix()); // +z green
    Renderer::renderMesh({ 0, 0, 0 }, { .9f, .9f, .9f }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix()); // unit cube
    Renderer::renderMesh({ -5, 3, 12 }, { .9f, .9f, .9f }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix()); // unit cube
    Renderer::renderMesh({ 5, 0, 0 }, { .9f, .9f, .9f }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix()); // unit cube
    Renderer::renderMesh({ 3.f, 80.f, 5.f }, { 10.f, 10.f, 10.f }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix());

  }

  void onImGuiRender() override {}
};