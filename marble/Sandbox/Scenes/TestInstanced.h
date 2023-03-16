#pragma once

#include <ranges>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>

#include "../Scene.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/Player.h"
#include "../../Utils/Mathf.h"
#include "../../Utils/MathIterators.h"
#include "../../World/Grass.h"

class TestInstancedScene : public Scene {
private:
  static constexpr unsigned int CHUNK_COUNT = 5;
  static constexpr unsigned int CHUNK_SIZE = 50;

  Renderer::Cubemap   m_skybox;
  Player              m_player, m_roguePlayer;
  bool                m_useRoguePlayer;
  Renderer::TerrainMesh m_terrain;
  Noise::ConcreteHeightMap m_heightmap;
  World::TerrainGrass m_grass;
  float               m_time;

public:
  TestInstancedScene()
    : m_skybox{
      "res/skybox/skybox_front.bmp", "res/skybox/skybox_back.bmp",
      "res/skybox/skybox_left.bmp",  "res/skybox/skybox_right.bmp",
      "res/skybox/skybox_top.bmp",   "res/skybox/skybox_bottom.bmp" },
      m_player(), m_roguePlayer(), m_useRoguePlayer(false), m_time(0)
  {
    m_player.setPostion({ 125, 10, 0 });
    m_player.setRotation(3.14f*3/4.f, 0);
    m_player.updateCamera();

    constexpr unsigned int worldSize = CHUNK_COUNT * CHUNK_SIZE;
    Noise::PerlinNoiseSettings perlinSettings;
    perlinSettings.scale = 100;
    perlinSettings.octaves = 3;
    m_heightmap = Noise::generateNoiseMap(worldSize, worldSize, perlinSettings);
    m_terrain.rebuildMesh(m_heightmap, { 0,0, worldSize,worldSize });

    auto terrainMaterial = std::make_shared<Renderer::Material>();
    terrainMaterial->shader = Renderer::getStandardMeshShader();
    m_terrain.setMaterial(terrainMaterial);

    // generate HD and LD chunk positions
    // they must be in terrain bounds and an LD chunk cannot overlap an HD chunk
    std::vector<glm::ivec2> hdChunks = Iterators::collect(
      std::views::all(Iterators::iterateOverCircle({ 0,0 }, 3)) |
      std::views::filter([this](glm::ivec2 p) { return m_terrain.hasChunk(p); }));
    std::vector<glm::ivec2> ldChunks = Iterators::collect(
      std::views::all(Iterators::iterateOverCircle({ 0,0 }, 5)) |
      std::views::filter([this, &hdChunks](glm::ivec2 p) { return m_terrain.hasChunk(p) && std::find(hdChunks.begin(), hdChunks.end(), p) == hdChunks.end(); }));
    m_grass = World::TerrainGrass(
      std::make_unique<World::FixedGrassChunks>(
        std::make_unique<World::TerrainGrassGenerator>(&m_heightmap),
        CHUNK_SIZE,
        hdChunks,
        ldChunks
      )
    );

    Renderer::getStandardMeshShader()->bind();
    Renderer::getStandardMeshShader()->setUniform1i("u_RenderChunks", 1);
    Renderer::Shader::unbind();
  }

  void step(float delta) override
  {
    m_grass.step(m_player.getCamera());
    m_time += delta;
    Player &activePlayer = m_useRoguePlayer ? m_roguePlayer : m_player;
    activePlayer.step(delta);
  }

  void onRender() override
  {
    Renderer::clear();
    Renderer::renderCubemap(m_player.getCamera(), m_skybox);

    const Renderer::Camera &renderCamera = (m_useRoguePlayer ? m_roguePlayer : m_player).getCamera();

    Renderer::renderMeshTerrain(renderCamera, m_terrain);

    if (m_useRoguePlayer)
      Renderer::renderDebugCameraOutline(renderCamera, m_player.getCamera());

    m_grass.render(renderCamera, m_player.getCamera(), m_time);
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

  CAMERA_IS_PLAYER(m_player);
};