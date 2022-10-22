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


  Renderer::Mesh m_cubeMesh;
  Renderer::Mesh m_waterMesh;
  Renderer::Mesh m_testMesh;

  Renderer::Texture m_rockTexture = Renderer::Texture( "res/textures/rock.jpg" );
  Renderer::Texture m_grassTexture = Renderer::Texture( "res/textures/grass5.jpg" );
  
  Renderer::TestUniform m_depthTestUniform;

  TerrainMeshGenerator::Terrain terrain;

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

    m_player.setPostion({ 100.f, 500.f, 0 });
    m_player.updateCamera();
    regenerateTerrain();
    m_waterMesh = Renderer::createPlaneMesh();
    m_cubeMesh = Renderer::createCubeMesh();
    m_sun.position = { 100,100,100 };
    m_water.position = { 50,24,50};
    m_grassTexture.bind(2U);
    m_rockTexture.bind(1U);
    GLint samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);
    Renderer::getStandardMeshShader().setUniform1f("u_Strenght", strength);
    m_testMesh = Renderer::loadMeshFromFile("res/meshes/house.obj");
    noiseMap = Noise::generateNoiseMap(w, h, scale, o, p, l, seed);
    terrain = TerrainMeshGenerator::generateTerrain(noiseMap, w, h, numberOfChunks, terrainHeight);
    m_depthTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_fogDamping", 3, .0001f);
    m_depthTestUniform.setValue(.003f, .01f, .013f);
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
  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());

    // TODO : fix chunkSize
    // TODO! : fix the texture issue in the standard mesh shader
    int i  = 0;
    m_rockTexture.bind(0);
    for (const auto& [position, chunk] : terrain.chunksPosition) {
           Renderer::renderMesh(glm::vec3{ position.x , 0.F, position.y} * m_mSize , glm::vec3(m_mSize), chunk.mesh, m_player.getCamera());
    }
    Renderer::renderMesh(m_sun.position, { 5,5,5 }, m_cubeMesh, m_player.getCamera());
    //Renderer::RenderMesh(m_Sun.position, { 100,100,100 }, m_testMesh, m_player.GetCamera().getViewProjectionMatrix());
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform3f("u_SunPos", m_sun.position.x, m_sun.position.y, m_sun.position.z);
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 0);
    if (m_renderChunks)
        Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 1);
    Renderer::getStandardMeshShader().unbind();



  }

  void onImGuiRender() override
  {
    if (ImGui::SliderInt("Width", (int*)&w, 10, 2000) + ImGui::SliderInt("Height", (int*)&h, 10, 2000) +
        ImGui::SliderFloat("Scale", &scale, 0, 50) + ImGui::SliderInt("Number of octaves", &o, 0, 10) +
        ImGui::SliderFloat("persistence", &p, 0, 1) + ImGui::SliderFloat("lacunarity", &l, 0, 50) +
        ImGui::SliderInt("seed", &seed, 0, 5) + ImGui::SliderFloat("Depth", &terrainHeight, 0, 100.f) +
        ImGui::SliderInt("chunksize", &numberOfChunks, 1, 16)) {
      regenerateTerrain();
    }

    ImGui::SliderFloat3("Sun position", &m_sun.position[0], -200, 200);
    ImGui::SliderFloat3("Water level", &m_water.position[0], -200, 200);
    
    if (ImGui::SliderFloat("Strength", &strength, 0, 2)) {
      Renderer::getStandardMeshShader().bind();
      Renderer::getStandardMeshShader().setUniform1f("u_Strength", strength);
    }
    
    ImGui::Checkbox("Fly", &m_playerIsFlying);
    ImGui::Checkbox("Render Chunks", &m_renderChunks);

    m_depthTestUniform.renderImGui();
  }
};