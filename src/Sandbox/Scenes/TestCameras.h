#pragma once

#include "../Scene.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/Player.h"

class TestCamerasScene : public Scene {
private:
  Renderer::Cubemap   m_skybox;
  std::vector<Player> m_players;
  int                 m_activePlayer;
  Renderer::Mesh      m_mesh1;
  Renderer::Texture   m_texture1;
public:
  TestCamerasScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
    m_mesh1(Renderer::loadMeshFromFile("res/meshes/house.obj")),
    m_players{},
    m_texture1("res/textures/rock.jpg"),
    m_activePlayer(0)
  {
    // player 1 (perspective)
    createPlayer({ 10, 10, 10 });

    // player 2 (perspective)
    createPlayer({ -10, 10, -10 });

    // player 3 (ortho)
    Player &p3 = createPlayer({ -10, 10, 10 });
    Renderer::OrthographicProjection p3proj;
    {
      float w = 16.f, h = w/(16.f/9.f);
      p3proj.left   = -w * .5f;
      p3proj.right  = +w * .5f;
      p3proj.bottom = -h * .5f;
      p3proj.top    = +h * .5f;
    }
    p3.getCamera().setProjection(p3proj);
    p3.updateCamera();
  }

  Player &createPlayer(const glm::vec3 &position)
  {
    Player &p = m_players.emplace_back();
    p.setPostion(position);
    p.updateCamera();
    p.getCamera().lookAt({ 0, 0, 0 }); // modifies the camera but not the player's rotation TODO remove Player::m_yaw&m_pitch because they already are in Player#m_camera
    p.setRotation(p.getCamera().getYaw(), p.getCamera().getPitch());
    p.updateCamera();
    return p;
  }

  void step(float delta) override
  {
    m_players[m_activePlayer].step(delta);
  }

  void onRender() override
  {
    Renderer::clear();
    Renderer::Camera &activeCamera = m_players[m_activePlayer].getCamera();
    Renderer::renderCubemap(activeCamera, m_skybox);
    m_texture1.bind();
    Renderer::renderMesh(activeCamera, { 0, 0, 0 }, { 1.f, 1.f, 1.f }, m_mesh1);
    for (size_t i = 0; i < m_players.size(); i++) {
      if(i != m_activePlayer)
        Renderer::renderDebugCameraOutline(activeCamera, m_players[i].getCamera());
    }
  }

  void onImGuiRender() override
  {
    ImGui::SliderInt("active player", &m_activePlayer, 0, (int)m_players.size()-1);
  }
};