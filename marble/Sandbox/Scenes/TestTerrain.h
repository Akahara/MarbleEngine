#pragma once

#include "../Scene.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/Player.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../Utils/AABB.h"

#ifndef NDEBUG
#define NDEBUG 0
#endif

class TestTerrainScene : public Scene {
private:

  //=====================================================================================================================//

    /* Basic scene stuff */
  Renderer::Cubemap m_skybox;
  Player            m_player;
  bool              m_playerIsFlying = true;
  float             realTime = 0;
  Player            m_roguePlayer;      // another player used to better visualize frustum culling
  bool              m_isRoguePlayerActive = false;

    /* Terrain generation stuff */
  Renderer::TerrainMesh      m_terrain;      // holds heightmap and chunksize
  Noise::PerlinNoiseSettings m_terrainData;  // < This holds default and nice configuration for the terrain
  Noise::ConcreteHeightMap   m_heightmap;
  bool                       m_isErosionEnabled = NDEBUG; // disable erosion by default when running in debug mode (because it's way too slow)
  Noise::ErosionSettings     m_erosionSettings;
  unsigned int               m_terrainSize = 20;

    /* Rendering stuff */
  Renderer::Frustum     m_frustum;
  
  bool                  m_renderChunks = 0;

  Renderer::TestUniform m_fogDampingTestUniform;
  Renderer::TestUniform m_grassSteepnessTestUniform;

  /* Other */

  struct Sun {
      glm::vec3 position{ 0.f };
      float strength = 1.25f;
  } m_sun;

  //=====================================================================================================================//

public:
  TestTerrainScene()
    : m_skybox{
      "res/skybox/skybox_front.bmp", "res/skybox/skybox_back.bmp",
      "res/skybox/skybox_left.bmp",  "res/skybox/skybox_right.bmp",
      "res/skybox/skybox_top.bmp",   "res/skybox/skybox_bottom.bmp" }
  {
    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();
    m_sun.position = { 100,100,100 };

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader()->bind();
    Renderer::getStandardMeshShader()->setUniform1iv("u_Textures2D", 8, samplers);
    Renderer::getStandardMeshShader()->setUniform1i("u_castShadows", 0);
    Renderer::Shader::unbind();

    auto terrainMaterial = std::make_shared<Renderer::Material>();
    terrainMaterial->shader = Renderer::getStandardMeshShader();
    terrainMaterial->textures[0] = std::make_shared<Renderer::Texture>("res/textures/rock.jpg");
    terrainMaterial->textures[1] = std::make_shared<Renderer::Texture>("res/textures/grass6.jpg");
    m_terrain.setMaterial(terrainMaterial);

    regenerateTerrain();

    m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());

    m_fogDampingTestUniform = Renderer::TestUniform(Renderer::getStandardMeshShader().get(), "u_fogDamping", 3, .0001f);
    m_fogDampingTestUniform.setValue(0, 0, .001f);
    m_grassSteepnessTestUniform = Renderer::TestUniform(Renderer::getStandardMeshShader().get(), "u_grassSteepness", 2, .01f);
    m_grassSteepnessTestUniform.setValue(.79f, 1.f);
  }

  void regenerateTerrain()
  {
    m_terrain.clearMesh();

    //{ // terrain from saved texture
    //  m_heightmap = Noise::loadNoiseMapFromFile("res/heightmaps/eroded.png");
    //  Noise::rescaleNoiseMap(&m_heightmap, 0, 1, 0, 100);
    //}

    { // simple terrain + erosion
      m_heightmap = Noise::generateNoiseMap(m_terrainSize, m_terrainSize, m_terrainData);
      if(m_isErosionEnabled)
        Noise::erode(&m_heightmap, m_erosionSettings);
    }

    m_terrain.rebuildMesh(m_heightmap, { 0,0, (float)m_terrainSize,(float)m_terrainSize });
  }

  void step(float delta) override
  {
      realTime += delta;
      (m_isRoguePlayerActive ? m_roguePlayer : m_player).step(delta);
      if (!m_playerIsFlying) {
          glm::vec3 pos = m_player.getPosition();
          pos.y = m_heightmap(pos.x, pos.z) + 1.f;
          m_player.setPostion(pos);
          m_player.updateCamera();
      }
    
      m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());
  }

  void onRender() override
  {
    Renderer::Camera &renderCamera = (m_isRoguePlayerActive ? m_roguePlayer : m_player).getCamera();

    Renderer::clear();
    Renderer::renderCubemap(renderCamera, m_skybox);

    if (DebugWindow::renderAABB()) {
      for (const auto &chunk : m_terrain.getChunks()) {
        const AABB &chunkAABB = chunk.worldBoundingBox;
        bool isVisible = m_frustum.isOnFrustum(chunkAABB);
        if (isVisible || m_isRoguePlayerActive)
          renderAABBDebugOutline(renderCamera, chunkAABB, isVisible ? glm::vec4{ 1,1,0,1 } : glm::vec4{ 1,0,0,1 });
      }
    }

    Renderer::renderMeshTerrain(renderCamera, m_terrain);

    if (m_isRoguePlayerActive) {
      Renderer::renderDebugCameraOutline(renderCamera, m_player.getCamera());
    }

    Renderer::getStandardMeshShader()->bind();
    Renderer::getStandardMeshShader()->setUniform3f("u_SunPos", m_sun.position.x, m_sun.position.y, m_sun.position.z);
    Renderer::getStandardMeshShader()->setUniform1f("u_Strength", m_sun.strength);
    Renderer::getStandardMeshShader()->setUniform1i("u_RenderChunks", m_renderChunks ? 1 : 0);
    Renderer::Shader::unbind();
  }

  static bool ImGuiTerrainDataSliders(Noise::PerlinNoiseSettings &data)
  {
    if (ImGui::Button("Reset")) {
      data = {};
      return true;
    }
    return
      ImGui::SliderFloat("Scale", &data.scale, 0, 50) +
      ImGui::SliderInt("Number of octaves", &data.octaves, 0, 10) +
      ImGui::SliderFloat("Frequency", &data.initialFrequency, .001f, 10) +
      ImGui::SliderFloat("Persistence", &data.persistence, 0, 1) +
      ImGui::SliderFloat("Lacunarity", &data.lacunarity, 0, 50) +
      ImGui::SliderInt("Seed", &data.seed, 0, 5) +
      ImGui::SliderFloat("Depth", &data.terrainHeight, 0, 100.f);
  }

  static bool ImGuiErosionSettingsSliders(Noise::ErosionSettings &settings)
  {
    if (ImGui::Button("Reset")) {
      settings = {};
      return true;
    }
    return
      ImGui::SliderInt("Erosion radius", &settings.erosionRadius, 1, 10) +
      ImGui::SliderFloat("Interia", &settings.inertia, 0, 1) +
      ImGui::SliderFloat("Sediment capacity", &settings.sedimentCapacityFactor, .01f, 100) +
      ImGui::SliderFloat("Min sediment capacity", &settings.minSedimentCapacity, 0, 3) +
      ImGui::SliderFloat("Erosion speed", &settings.erodeSpeed, 0, 1) +
      ImGui::SliderFloat("Deposit speed", &settings.depositSpeed, 0, 1) +
      ImGui::SliderFloat("Gravity", &settings.gravity, 0, 5) +
      ImGui::SliderFloat("Initial water volume", &settings.initialWaterVolume, 0, 3) +
      ImGui::SliderFloat("Initial speed", &settings.initialSpeed, 0, 3) +
      ImGui::SliderInt("Max droplet lifetime", &settings.maxDropletLifetime, 1, 100) +
      ImGui::DragInt("Droplet count", (int*)&settings.dropletCount, 0, (int)1E9);
  }

  void onImGuiRender() override
  {
    if (ImGui::CollapsingHeader("Terrain Settings")) {
      int regenerate = 0;

      regenerate += ImGui::SliderInt("Size (chunk aligned)", (int *)&m_terrainSize, 1, 1000);
      regenerate += ImGui::Checkbox("Erosion", &m_isErosionEnabled);
      if (!m_isErosionEnabled) ImGui::BeginDisabled();
      regenerate += ImGuiErosionSettingsSliders(m_erosionSettings);
      if(!m_isErosionEnabled) ImGui::EndDisabled();
      ImGui::Text("Terrain");
      regenerate += ImGuiTerrainDataSliders(m_terrainData);

      if(regenerate)
        regenerateTerrain();
    }

    ImGui::SliderFloat3("Sun position", &m_sun.position[0], -200, 200);
    ImGui::SliderFloat("Sun strength", &m_sun.strength, 0, 3);
    ImGui::Checkbox("Fly", &m_playerIsFlying);
    ImGui::Checkbox("Render Chunks", &m_renderChunks);
    ImGui::Checkbox("Use rogue player", &m_isRoguePlayerActive);
    glm::vec3 playerPos = m_player.getPosition();
    if (ImGui::DragFloat3("Player position", &playerPos.x, .1f)) {
      m_player.setPostion(playerPos);
      m_player.updateCamera();
    }

    m_fogDampingTestUniform.renderImGui();
    m_grassSteepnessTestUniform.renderImGui();
  }

  CAMERA_IS_PLAYER(m_player);
};