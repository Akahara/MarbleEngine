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
  Renderer::Texture m_terrainTexture;
  Renderer::Mesh    m_terrainMesh;
  bool              m_playerIsFlying = true;
  int w = 200, h = 200;
  float scale = 27.6f;
  float terrainHeight = 120.f;
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

  TerrainMeshGenerator::Terrain terrain;

  struct Sun {

      glm::vec3 position;

  } m_Sun;

  struct Water {

      glm::vec3 position;

  } m_Water;


public:
  TestTerrainScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
      
  {
    m_player.setPostion({ 100.f, 500.f, 0 });
    m_player.UpdateCamera();
    RegenerateTerrain();
    //m_terrainMesh.AddTexture(m_grassTexture);
    m_waterMesh = Renderer::CreatePlaneMesh();
    m_cubeMesh = Renderer::CreateCubeMesh();
    m_Sun.position = { 100,100,100 };
    m_Water.position = { 50,24,50};
    m_grassTexture.Bind(2U);
    m_rockTexture.Bind(1U);
    Renderer::getShader().Bind();
    GLint samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getShader().SetUniform1iv("u_Textures2D", 8, samplers);
    Renderer::getShader().SetUniform1f("u_Strenght", strength);
    m_testMesh = Renderer::LoadMeshFromFile("res/meshes/house.obj");
    noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);
    terrain = TerrainMeshGenerator::generateTerrain(noiseMap, w, h, 8);

  }

  void RegenerateTerrain()
  {
    free(noiseMap);
    noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);
    m_heightmap.setHeights(w, h, noiseMap);
    terrain = TerrainMeshGenerator::generateTerrain(noiseMap, w, h, 8);
    //m_terrainTexture = MapUtilities::genTextureFromHeightmap(m_heightmap);
  }

  void Step(float delta) override
  {
      realTime += delta;
      m_player.Step(delta);
      Renderer::getShader().Bind();
      Renderer::getShader().Unbind();
    if (!m_playerIsFlying) {
      glm::vec3 pos = m_player.GetPosition();
      pos.y = m_heightmap.getHeightLerp(pos.x, pos.z) + 1.f;
      m_player.setPostion(pos);
      m_player.UpdateCamera();
    }
  }

  void OnRender() override
  {
    Renderer::CubemapRenderer::DrawCubemap(m_skybox, m_player.GetCamera(), m_player.GetPosition());

    // TODO : fix chunkSize
    int i  = 0;
    m_terrainTexture.Bind();
    for (const auto& [position, chunk] : terrain.chunksPosition) {
           Renderer::RenderMesh(glm::vec3{ position.x , 300.f, position.y}, glm::vec3(1), chunk.mesh, m_player.GetCamera().getViewProjectionMatrix());
    }
    Renderer::RenderMesh(m_Sun.position, { 5,5,5 }, m_cubeMesh, m_player.GetCamera().getViewProjectionMatrix());
    //Renderer::RenderMesh(m_Sun.position, { 100,100,100 }, m_testMesh, m_player.GetCamera().getViewProjectionMatrix());
    Renderer::getShader().Bind();
    Renderer::getShader().SetUniform3f("u_SunPos", m_Sun.position.x, m_Sun.position.y, m_Sun.position.z);
    Renderer::getShader().Unbind();



  }

  void OnImGuiRender() override
  {
    //std::cout << "imgui step" << std::endl;
    if (ImGui::SliderInt("Width", &w, 10, 2000) + ImGui::SliderInt("Height", &h, 10, 2000) +
        ImGui::SliderFloat("Scale", &scale, 0, 50) + ImGui::SliderInt("Number of octaves", &o, 0, 10) +
        ImGui::SliderFloat("persistence", &p, 0, 1) + ImGui::SliderFloat("lacunarity", &l, 0, 50) +
        ImGui::SliderInt("seed", &seed, 0, 5) + ImGui::SliderFloat("Depth", &terrainHeight, 0, 100.f)) {
      RegenerateTerrain();
    }

    ImGui::SliderFloat3("Sun position", &m_Sun.position[0], -200, 200);
    ImGui::SliderFloat3("Water level", &m_Water.position[0], -200, 200);
    
    if (ImGui::SliderFloat("Strength", &strength, 0, 2)) {
        Renderer::getShader().SetUniform1f("u_Strenght", strength);
    }
    
    ImGui::Checkbox("Fly", &m_playerIsFlying);
  }
};