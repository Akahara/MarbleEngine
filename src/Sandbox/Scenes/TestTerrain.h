#pragma once

#include "../Scene.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Mesh.h"
#include "../../World/Player.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../Utils/AABB.h"
#include "../Scenes/TestShadows.h"

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
  Player            m_roguePlayer; // another player used to better visualize frustum culling
  bool              m_isRoguePlayerActive = false;

    /* Terrain generation stuff */
  Terrain::Terrain      m_terrain;      // holds heightmap and chunksize
  Terrain::TerrainData  m_terrainData;  // < This holds default and nice configuration for the terrain
  bool                  m_isErosionEnabled = NDEBUG; // disable erosion by default when running in debug mode (because it's way too slow)
  Noise::ErosionSettings m_erosionSettings;
  unsigned int          m_terrainSizeInChunks = 20;
  int                   m_chunkSize = 10;

    /* Rendering stuff */
  Renderer::Frustum     m_frustum;
  
  Renderer::Texture     m_rockTexture = Renderer::Texture( "res/textures/rock.jpg" );
  Renderer::Texture     m_grassTexture = Renderer::Texture( "res/textures/grass6.jpg" );

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
    m_rockTexture.bind(0);
    m_grassTexture.bind(1);

    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();
    m_sun.position = { 100,100,100 };

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);
    Renderer::getStandardMeshShader().setUniform1i("u_castShadows", 0);

    regenerateTerrain();

    m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());

    m_fogDampingTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_fogDamping", 3, .0001f);
    //m_fogDampingTestUniform.setValue(.003f, .01f, .013f);
    m_fogDampingTestUniform.setValue(0, 0, .001f);
    m_grassSteepnessTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_grassSteepness", 2, .01f);
    m_grassSteepnessTestUniform.setValue(.79f, 1.f);
  }

  void regenerateTerrain()
  {
    //{ // simple terrain generation
    //  m_terrain = Terrain::generateTerrain(
    //    m_terrainData,
    //    m_terrainWidthInChunks,
    //    m_terrainHeightInChunks, 
    //    m_chunkSize);
    //}

    //{ // terrain from saved texture
    //  unsigned int noiseMapWidth, noiseMapHeight;
    //  float *noiseMap = Noise::loadNoiseMapFromFile("res/heightmaps/eroded.png", &noiseMapWidth, &noiseMapHeight);
    //  Noise::rescaleNoiseMap(noiseMap, noiseMapWidth, noiseMapHeight, 0, 1, 0, 100);
    //  ConcreteHeightMap *heightMap = new ConcreteHeightMap(noiseMapWidth, noiseMapHeight, noiseMap);
    //  m_terrain = Terrain::generateTerrain(heightMap, 1, 1, noiseMapWidth - 3);
    //}

    { // simple terrain + erosion
      unsigned int noiseMapSize = 3 + m_chunkSize * m_terrainSizeInChunks;
      float *noiseMap = Noise::generateNoiseMap(noiseMapSize,
                                                noiseMapSize,
                                                m_terrainData.scale,
                                                m_terrainData.octaves,
                                                m_terrainData.persistence,
                                                m_terrainData.initialFrequency,
                                                m_terrainData.lacunarity,
                                                m_terrainData.seed);
      if(m_isErosionEnabled)
        Noise::erode(noiseMap, noiseMapSize, m_erosionSettings);
      Noise::rescaleNoiseMap(noiseMap, noiseMapSize, noiseMapSize, 0, 1, 0, m_terrainData.terrainHeight);
      Terrain::HeightMap *heightMap = new Terrain::ConcreteHeightMap(noiseMapSize, noiseMapSize, noiseMap);
      m_terrain = Terrain::generateTerrain(heightMap, m_terrainSizeInChunks, m_terrainSizeInChunks, m_chunkSize);
    }
  }

  void step(float delta) override
  {
      realTime += delta;
      (m_isRoguePlayerActive ? m_roguePlayer : m_player).step(delta);
      if (!m_playerIsFlying) {
          glm::vec3 pos = m_player.getPosition();
          pos.y = m_terrain.getHeightMap().getHeightLerp(pos.x, pos.z) + 1.f;
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

    m_rockTexture.bind(0);
    m_grassTexture.bind(1);

    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();
      bool isVisible = m_frustum.isOnFrustum(chunkAABB);

      if (DebugWindow::renderAABB() && (isVisible || m_isRoguePlayerActive))
        renderAABBDebugOutline(renderCamera, chunkAABB, isVisible ? glm::vec4{ 1,1,0,1 } : glm::vec4{ 1,0,0,1 });

      if (isVisible) {
        Renderer::renderMesh(renderCamera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());
      }
    }

    if (m_isRoguePlayerActive) {
      Renderer::renderDebugCameraOutline(renderCamera, m_player.getCamera());
    }

    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform3f("u_SunPos", m_sun.position.x, m_sun.position.y, m_sun.position.z);
    Renderer::getStandardMeshShader().setUniform1f("u_Strength", m_sun.strength);
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", m_renderChunks ? 1 : 0);
    Renderer::getStandardMeshShader().unbind();
  }

  static bool ImGuiTerrainDataSliders(Terrain::TerrainData &data)
  {
    if (ImGui::Button("Reset")) {
      data = {};
      return true;
    }
    return
      ImGui::SliderFloat("Scale", &data.scale, 0, 50) +
      ImGui::SliderInt("Number of octaves", &data.octaves, 0, 10) +
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
      bool regenerate = false;

      regenerate += ImGui::SliderInt("Size (in chunks)", (int *)&m_terrainSizeInChunks, 1, 100);
      regenerate += ImGui::SliderInt("Chunk size", &m_chunkSize, 1, 128);
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
};