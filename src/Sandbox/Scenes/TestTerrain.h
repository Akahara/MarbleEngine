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
  float terrainHeight = 15.f;
  int o = 4;
  float p = 0.3f, l = 3.18f;
  int seed = 5;
public:
  TestTerrainScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
  {
    m_player.setPostion({ 0.f, 30.f, 0 });
    m_player.UpdateCamera();
    RegenerateTerrain();
  }

  void RegenerateTerrain()
  {
    float *noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);
    m_heightmap.setHeights(w, h, noiseMap);
    m_terrainMesh = TerrainMeshGenerator::generateMesh(m_heightmap, { w, terrainHeight, h });
    m_terrainTexture = MapUtilities::genTextureFromHeightmap(m_heightmap);
  }

  void Step(float delta) override
  {
    m_player.Step(delta);
    if (!m_playerIsFlying) {
      glm::vec3 pos = m_player.GetPosition();
      pos.y = m_heightmap.getHeightLerp(pos.x, pos.z) + 1.f;
      m_player.setPostion(pos);
      m_player.UpdateCamera();
    }
  }

  void OnRender() override
  {
    Renderer::Renderer::Clear();
    Renderer::CubemapRenderer::DrawCubemap(m_skybox, m_player.GetCamera(), m_player.GetPosition());
    m_terrainTexture.Bind();
    Renderer::RenderMesh({}, { 1.f, 1.f, 1.f }, m_terrainMesh, m_player.GetCamera().getViewProjectionMatrix());
  }

  void OnImGuiRender() override
  {
    if (ImGui::SliderInt("Width", &w, 10, 2000) + ImGui::SliderInt("Height", &h, 10, 2000) +
        ImGui::SliderFloat("Scale", &scale, 0, 50) + ImGui::SliderInt("Number of octaves", &o, 0, 10) +
        ImGui::SliderFloat("persistence", &p, 0, 1) + ImGui::SliderFloat("lacunarity", &l, 0, 50) +
        ImGui::SliderInt("seed", &seed, 0, 5) + ImGui::SliderFloat("Depth", &terrainHeight, 0, 100.f)) {
      RegenerateTerrain();
    }

    ImGui::Checkbox("Fly", &m_playerIsFlying);
  }
};