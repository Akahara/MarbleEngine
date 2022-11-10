#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/Player.h"
#include "../../Utils/Mathf.h"
#include "../../Utils/MathIterators.h"
#include "../../World/Grass.h"

class TestInstancedScene : public Scene {
private:
  Renderer::Cubemap   m_skybox;
  Player              m_player, m_roguePlayer;
  bool                m_useRoguePlayer;
  Renderer::Texture   m_texture1;
  TerrainMeshGenerator::Terrain m_terrain;
  GrassRenderer       m_grass;

public:
  TestInstancedScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
      m_texture1("res/textures/rock.jpg"),
      m_player(), m_roguePlayer(), m_useRoguePlayer(false)
  {
    m_player.setPostion({ 125, 10, 0 });
    m_player.setRotation(3.14f*3/4.f, 0);
    m_player.updateCamera();

    TerrainMeshGenerator::TerrainData terrainData{};
    terrainData.scale = 100.f;
    terrainData.octaves = 3;
    terrainData.seed = 0;
    m_terrain = TerrainMeshGenerator::generateTerrain(
      terrainData,
      CHUNK_COUNT_X, CHUNK_COUNT_Y,
      CHUNK_SIZE
    );
    m_grass = GrassRenderer(m_terrain, GrassChunks::positionToGrassChunk(m_player.getCamera().getPosition()));

    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 1);
  }

  void step(float delta) override
  {
    m_grass.step(m_player.getCamera());
    Player &activePlayer = m_useRoguePlayer ? m_roguePlayer : m_player;
    activePlayer.step(delta);
  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());

    const Renderer::Camera &renderCamera = (m_useRoguePlayer ? m_roguePlayer : m_player).getCamera();
    m_texture1.bind(0);
    m_texture1.bind(1);
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      Renderer::renderMesh(glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh(), renderCamera);

      if (DebugWindow::renderAABB())
        renderAABBDebugOutline(renderCamera, chunk.getMesh().getBoundingBox());
    }

    if (m_useRoguePlayer) {
      Renderer::renderDebugCameraOutline(renderCamera, m_player.getCamera());
    }

    m_grass.render(renderCamera, m_player.getCamera());
  }

  void onImGuiRender() override
  {
    if (ImGui::Begin(">")) {
      ImGui::Checkbox("rogue player", &m_useRoguePlayer);
      glm::vec3 pp = m_player.getPosition();
      glm::vec2 pr = m_player.getRotation();
      ImGui::DragFloat3("player position", glm::value_ptr(pp), .1f);
      ImGui::DragFloat2("player rotation", glm::value_ptr(pr), .1f);
      m_player.setPostion(pp);
      m_player.setRotation(pr.x, pr.y);
      m_player.updateCamera();
    }
    ImGui::End();
  }
};