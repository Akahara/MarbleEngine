#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Mesh.h"
#include "../../World/Player.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/MapUtilities.h"
#include "../../World/TerrainGeneration/Noise.h"

#include "../../Utils/AABB.h"
#include "../Scenes/TestShadows.h"

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
  Renderer::Mesh                    m_terrainMesh;
  TerrainMeshGenerator::Terrain     m_terrain;      // holds heightmap and chunksize
  TerrainMeshGenerator::TerrainData m_terrainData;  // < This holds default and nice configuration for the terrain
  unsigned int                      m_terrainWidthInChunks = 10, m_terrainHeightInChunks = 10;
  int                               m_chunkSize = 16;


    /* Rendering stuff */
  Renderer::Frustum     m_frustum;
  
  Renderer::Texture     m_rockTexture = Renderer::Texture( "res/textures/rock.jpg" );
  Renderer::Texture     m_grassTexture = Renderer::Texture( "res/textures/grass6.jpg" );

  bool                  m_renderChunks = 0;

  Renderer::TestUniform m_depthTestUniform;
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
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
  {
    m_rockTexture.bind(0);
    m_grassTexture.bind(1);

    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();
    regenerateTerrain();
    m_sun.position = { 100,100,100 };

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);

    Renderer::getStandardMeshShader().setUniform1f("u_Strength", m_sun.strength);

    regenerateTerrain();

    m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());

    m_depthTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_fogDamping", 3, .0001f);
    m_depthTestUniform.setValue(.003f, .01f, .013f);
    m_grassSteepnessTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_grassSteepness", 2, .01f);
    m_grassSteepnessTestUniform.setValue(.79f, 1.f);
  }

  void regenerateTerrain()
  {
    m_terrain = TerrainMeshGenerator::generateTerrain(
      m_terrainData,
      m_terrainWidthInChunks,
      m_terrainHeightInChunks, 
      m_chunkSize);
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

    Renderer::Renderer::clear();
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
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 0);
    if (m_renderChunks)
      Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 1);
    Renderer::getStandardMeshShader().unbind();
  }

  void onImGuiRender() override
  {
    if (ImGui::SliderInt("Width", (int*)&m_terrainWidthInChunks, 2, 100) + ImGui::SliderInt("Height", (int*)& m_terrainHeightInChunks, 2, 100) +
        ImGui::SliderFloat("Scale", &m_terrainData.scale, 0, 50) + ImGui::SliderInt("Number of octaves", &m_terrainData.octaves, 0, 10) +
        ImGui::SliderFloat("persistence", &m_terrainData.persistence, 0, 1) + ImGui::SliderFloat("lacunarity", &m_terrainData.lacunarity, 0, 50) +
        ImGui::SliderInt("seed", &m_terrainData.seed, 0, 5) + ImGui::SliderFloat("Depth", &m_terrainData.terrainHeight, 0, 100.f) +
        ImGui::SliderInt("chunksize", &m_chunkSize, 1, 16)) {
      regenerateTerrain();
    }

    ImGui::SliderFloat3("Sun position", &m_sun.position[0], -200, 200);
    ImGui::Checkbox("Fly", &m_playerIsFlying);
    ImGui::Checkbox("Render Chunks", &m_renderChunks);
    ImGui::Checkbox("Use rogue player", &m_isRoguePlayerActive);
    glm::vec3 playerPos = m_player.getPosition();
    if (ImGui::DragFloat3("Player position", &playerPos.x, .1f)) {
      m_player.setPostion(playerPos);
      m_player.updateCamera();
    }

    m_depthTestUniform.renderImGui();
    m_grassSteepnessTestUniform.renderImGui();
  }
};