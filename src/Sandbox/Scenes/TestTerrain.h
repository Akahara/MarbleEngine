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
  TerrainMeshGenerator::TerrainData m_terrainData; // < This holds default and nice configuration for the terrain
  int                               m_numberOfChunks = 8;


    /* Rendering stuff */
  Renderer::Frustum     m_frustum;
  
  Renderer::Texture     m_rockTexture = Renderer::Texture( "res/textures/rock.jpg" );
  Renderer::Texture     m_grassTexture = Renderer::Texture( "res/textures/grass6.jpg" );

  bool                  m_renderChunks = 0;

  Renderer::TestUniform m_depthTestUniform;



  /* Other */

  float test = 1;

  Renderer::Mesh        m_treeMesh = Renderer::loadMeshFromFile("res/meshes/Tree.obj"); // <! CHANGER CE PUTAIN DE SOL
  Renderer::Mesh        m_grassBlade = Renderer::loadMeshFromFile("res/meshes/blade.obj"); 

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

    Renderer::getStandardMeshShader().setUniform1f("u_Strenght", m_sun.strength);

    m_terrain = TerrainMeshGenerator::generateTerrain(m_terrainData, m_numberOfChunks);

    m_frustum = Renderer::Frustum::createFrustumFromCamera(
        m_player.getCamera(),
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().aspect,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().fovy,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zNear,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zFar
    );

    m_depthTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_fogDamping", 3, .0001f);
    m_depthTestUniform.setValue(.003f, .01f, .013f);
  }

  void regenerateTerrain()
  {

        free(m_terrainData.noiseMap);

        m_terrainData.noiseMap = Noise::generateNoiseMap(
            m_terrainData.width,
            m_terrainData.height,
            m_terrainData.scale,
            m_terrainData.octaves,
            m_terrainData.persistence,
            m_terrainData.lacunarity,
            m_terrainData.seed
        );


        m_terrain.heightMap.setHeights(
            m_terrainData.width,
            m_terrainData.height,
            m_terrainData.noiseMap
        );

        m_terrain = TerrainMeshGenerator::generateTerrain(m_terrainData, m_numberOfChunks);

  }

  void step(float delta) override
  {
      realTime += delta;
      (m_isRoguePlayerActive ? m_roguePlayer : m_player).step(delta);
      if (!m_playerIsFlying) {
          glm::vec3 pos = m_player.getPosition();
          pos.y = m_terrain.heightMap.getHeightLerp(pos.x, pos.z) + 1.f;
          m_player.setPostion(pos);
          m_player.updateCamera();
      }
    
      m_frustum = Renderer::Frustum::createFrustumFromCamera( // TODO #createFrustumFromCamera can retrieve the projection itself, instead of passing the 4 arguments here
          m_player.getCamera(),
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().aspect,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().fovy,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zNear,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zFar
      );
    

  }

  void onRender() override
  {
    Renderer::Camera &renderCamera = (m_isRoguePlayerActive ? m_roguePlayer : m_player).getCamera();

    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, renderCamera);

    m_rockTexture.bind(0);
    m_grassTexture.bind(1);

    for (const auto& [position, chunk] : m_terrain.chunksPosition) {
      const AABB &chunkAABB = chunk.mesh.getBoundingBox();
      bool isVisible = Renderer::Frustum::isOnFrustum(m_frustum, chunkAABB);

      if (DebugWindow::renderAABB() && (isVisible || m_isRoguePlayerActive))
        renderAABBDebugOutline(renderCamera, chunkAABB, isVisible ? glm::vec4{ 1,1,0,1 } : glm::vec4{ 1,0,0,1 });

      if (isVisible) {
        Renderer::renderMesh(glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.mesh, renderCamera);
      }
    }

    if (m_isRoguePlayerActive) {
      Renderer::renderDebugCameraOutline(renderCamera, m_player.getCamera());
    }

    //Renderer::renderMesh(glm::vec3{ 100.f ,m_terrain.heightMap.getHeight(100.f, 100.f) * m_terrainData.terrainHeight, 100.F}, glm::vec3(2), m_treeMesh, m_player.getCamera(), true);
    /*
    for (unsigned int i = 0; i < 100; i++) {
        for (unsigned int j = 0; j < 100; j++) {

            if (Renderer::Frustum::isOnFrustum(m_frustum, m_grassBlade.getBoundingBox())) {
                Renderer::renderMesh(glm::vec3{ i ,m_terrain.heightMap.getHeight(i, j) * m_terrainData.terrainHeight, j }, glm::vec3(10), m_grassBlade, m_player.getCamera(), true);
            }

            Renderer::renderMesh(glm::vec3{ i ,m_terrain.heightMap.getHeight(i, j) * m_terrainData.terrainHeight, j }, glm::vec3(10), m_grassBlade, m_player.getCamera(), true);
        }
    }
    */

    // renderAABBDebugOutline(m_player.getCamera(), m_treeMesh.getBoundingBox());

    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform3f("u_SunPos", m_sun.position.x, m_sun.position.y, m_sun.position.z);
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 0);
    if (m_renderChunks)
        Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 1);
    Renderer::getStandardMeshShader().unbind();

  }

  void onImGuiRender() override
  {
      
    if (ImGui::SliderInt("Width", (int*)&m_terrainData.width, 10, 2000) + ImGui::SliderInt("Height", (int*)& m_terrainData.height, 10, 2000) +
        ImGui::SliderFloat("Scale", &m_terrainData.scale, 0, 50) + ImGui::SliderInt("Number of octaves", &m_terrainData.octaves, 0, 10) +
        ImGui::SliderFloat("persistence", &m_terrainData.persistence, 0, 1) + ImGui::SliderFloat("lacunarity", &m_terrainData.lacunarity, 0, 50) +
        ImGui::SliderInt("seed", &m_terrainData.seed, 0, 5) + ImGui::SliderFloat("Depth", &m_terrainData.terrainHeight, 0, 100.f) +
        ImGui::SliderInt("chunksize", &m_numberOfChunks, 1, 16)) {
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
  }
};