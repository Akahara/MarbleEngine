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
  Renderer::Cubemap m_skybox;
  Player            m_player;
  HeightMap         m_heightmap;
  Renderer::Mesh    m_terrainMesh;
  bool              m_playerIsFlying = true;
  unsigned int w = 200, h = 200;
  float scale = 27.6f;
  float terrainHeight = 20.f;
  int o = 4;
  float p = 0.3f, l = 3.18f;
  int seed = 5;
  float strength = 1.25f;

  float realTime = 0;
  float* noiseMap;

  glm::vec3 m_CowSize{5};

  AABB m_aabbSun;

  Renderer::Mesh m_cubeMesh;
  Renderer::Mesh m_waterMesh;
  Renderer::Mesh m_testMesh;
  
  Renderer::Texture m_rockTexture = Renderer::Texture( "res/textures/rock.jpg" );
  Renderer::Texture m_grassTexture = Renderer::Texture( "res/textures/grass6.jpg" );

  TerrainMeshGenerator::Terrain terrain;
  std::vector<AABB> m_aabbs;
  Renderer::Frustum m_frustum;
  Renderer::Frustum m_frustumTemp;

  Renderer::Camera m_camTemp;

  float m_mSize = 1;
  bool m_renderChunks = 0;
  int numberOfChunks = 8;

  struct Sun {

      glm::vec3 position;

  } m_sun;

  struct Water {

      glm::vec3 position;

  } m_water;


public:
  TestTerrainScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
  {



      m_rockTexture.bind(0);
      m_grassTexture.bind(1);


    m_player.setPostion({ 100.f, 500.f, 0 });
    m_player.updateCamera();
    regenerateTerrain();
    m_waterMesh = Renderer::createPlaneMesh();
    m_cubeMesh = Renderer::createCubeMesh();
    m_sun.position = { 100,100,100 };
    m_water.position = { 50,24,50};


    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);

    Renderer::getStandardMeshShader().setUniform1f("u_Strenght", strength);
    m_testMesh = Renderer::loadMeshFromFile("res/meshes/cow.obj");
    noiseMap = Noise::generateNoiseMap(w, h, scale, o, p, l, seed);
    terrain = TerrainMeshGenerator::generateTerrain(noiseMap, w, h, numberOfChunks, terrainHeight);

    unsigned int chunkSize = std::min(terrain.heightMap.getMapWidth() / numberOfChunks, terrain.heightMap.getMapHeight() / numberOfChunks);


    /*
    for (const auto& [position, chunk] : terrain.chunksPosition) {

        // make aabb

        AABB aabb = AABB({ position.x, 0.f, position.y }, glm::vec3(chunkSize));
        m_aabbs.push_back(aabb);
    }

    m_aabbSun = AABB{ m_sun.position ,{5,5,5}};

    m_frustum = Renderer::Frustum::createFrustumFromCamera(
        m_player.getCamera(),
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().aspect,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().fovy,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zNear,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zFar
    );

    m_frustumTemp = m_frustum;



    m_camTemp = m_player.getCamera();
    */


  }

  void regenerateTerrain()
  {
    free(noiseMap);
    noiseMap = Noise::generateNoiseMap(w, h, scale, o, p, l, seed);
    m_heightmap.setHeights(w, h, noiseMap);
    terrain = TerrainMeshGenerator::generateTerrain(noiseMap, w, h, numberOfChunks, terrainHeight);
    //m_terrainTexture = MapUtilities::genTextureFromHeightmap(m_heightmap);
  }

  void step(float delta) override
  {
      realTime += delta;
      m_player.step(delta);
      if (!m_playerIsFlying) {
          glm::vec3 pos = m_player.getPosition();
          pos.y = m_heightmap.getHeightLerp(pos.x, pos.z) + 1.f;
          m_player.setPostion(pos);
          m_player.updateCamera();
      }
    /*
      m_aabbSun.setOrigin(m_sun.position + glm::vec3{-m_CowSize.x, m_CowSize.y, -m_CowSize.z});
      m_aabbSun.setSize({-m_CowSize.x, m_CowSize.y, -m_CowSize.z});

      m_frustum = Renderer::Frustum::createFrustumFromCamera(
          m_player.getCamera(),
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().aspect,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().fovy,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zNear,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zFar
      );
    */

  }

  void onRender() override
  {
    Renderer::clearDebugData();
    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera(), m_player.getPosition());

    // TODO! : fix the texture issue in the standard mesh shader


    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);
    Renderer::getStandardMeshShader().unbind();
    m_rockTexture.bind(0);
    m_grassTexture.bind(1);

    
  
    //Renderer::renderMesh(m_sun.position, { 5,5,5 }, m_cubeMesh, m_player.getCamera().getViewProjectionMatrix());
    Renderer::renderMesh(m_sun.position, m_CowSize, m_testMesh, m_player.getCamera().getViewProjectionMatrix());

    Renderer::renderMesh({0.f, 0.f, 0.f}, glm::vec3(20), m_cubeMesh, m_player.getCamera().getViewProjectionMatrix());
    Renderer::renderMesh({30.f, 0.f, 0.f}, glm::vec3(20), m_cubeMesh, m_player.getCamera().getViewProjectionMatrix());
    /*
    for (const auto& aabb : m_aabbs)
        renderAABBDebugOutline(m_player.getCamera(), aabb);

    if (Renderer::Frustum::isOnFrustum(m_frustum, m_aabbSun)) {
        renderAABBDebugOutline(m_player.getCamera(), m_aabbSun);
    }

    for (const auto& [position, chunk] : terrain.chunksPosition) {
           Renderer::renderMesh(glm::vec3{ position.x , 0.F, position.y} * m_mSize , glm::vec3(m_mSize), chunk.mesh, m_player.getCamera().getViewProjectionMatrix());
    }
    */

    //Renderer::renderDebugPerspectiveCameraOutline(m_player.getCamera(), m_camTemp);
    //Renderer::renderDebugFrustumOutline(m_player.getCamera(), m_frustumTemp);



    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform3f("u_SunPos", m_sun.position.x, m_sun.position.y, m_sun.position.z);
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 0);
    if (m_renderChunks)
        Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 1);
    Renderer::getStandardMeshShader().unbind();

    Renderer::showDebugData();
  }

  void onImGuiRender() override
  {
      /*
    if (ImGui::SliderInt("Width", (int*)&w, 10, 2000) + ImGui::SliderInt("Height", (int*)&h, 10, 2000) +
        ImGui::SliderFloat("Scale", &scale, 0, 50) + ImGui::SliderInt("Number of octaves", &o, 0, 10) +
        ImGui::SliderFloat("persistence", &p, 0, 1) + ImGui::SliderFloat("lacunarity", &l, 0, 50) +
        ImGui::SliderInt("seed", &seed, 0, 5) + ImGui::SliderFloat("Depth", &terrainHeight, 0, 100.f) +
        ImGui::SliderInt("chunksize", &numberOfChunks, 1, 16)) {
      regenerateTerrain();
    }
      */

    ImGui::SliderFloat3("Sun position", &m_sun.position[0], -200, 200);
    ImGui::SliderFloat3("Cow size", &m_CowSize[0], -5, 15);
    ImGui::SliderFloat3("Water level", &m_water.position[0], -200, 200);
    
    if (ImGui::SliderFloat("Strength", &strength, 0, 2)) {
      Renderer::getStandardMeshShader().bind();
      Renderer::getStandardMeshShader().setUniform1f("u_Strength", strength);
    }
    
    if (ImGui::Button("refresh camera")) {
        m_camTemp = m_player.getCamera();

        m_frustumTemp = m_frustum;
    }
    ImGui::Checkbox("Fly", &m_playerIsFlying);
    ImGui::Checkbox("Render Chunks", &m_renderChunks);
  }
};