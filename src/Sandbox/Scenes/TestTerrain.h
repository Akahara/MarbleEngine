#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/TempRenderer.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MapGenerator.h"
#include "../../World/Player.h"

class TestTerrainScene : public Scene {
private:
  Renderer::Cubemap m_skybox;
  Player            m_player;
  HeightMap         m_heightmap;
  bool              m_playerIsFlying = true;
  int w = 200, h = 200;
  float scale = 27.6f;
  int o = 4;
  float p = 0.3f, l = 3.18f;
  int seed = 5;
  unsigned int id;
public:
  TestTerrainScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
  {
    float *noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);
    for (int i = 0; i < w * h; i++) // TODO remove and add an amplitude slider to GenerateNoiseMap
      noiseMap[i] *= 15.f;             // TODO also color the terrain differently based on that amplitude
    MapGenerator mapGen(w, h, scale, o, p, l, seed); // not sure of how the generator intervenes
    m_player.setPostion({ 0.f, 30.f, 0 });
    m_heightmap.setHeights(w, h, noiseMap);
    id = mapGen.GenerateTextureMap(w, h, noiseMap);
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
    Renderer::CubemapRenderer::DrawCubemap(m_skybox, m_player.GetCamera(), m_player.GetPosition());
    // TODO FIX - currently RenderGrid takes a "quadsPerSide" parameter which should realy be "gridWidth" *and* "gridHeight"
    // and the scale parameter should be computed based on the grid size
    // and the mesh should be generated based on the heightmap
    float scale = 1.f*w;
    int quadsPerSide = w - 1;
    TempRenderer::RenderCube({}, { 1.f, 1.f, 1.f }, { 1.f, 0.f, 1.f }, m_player.GetCamera().getViewProjectionMatrix());
    TempRenderer::RenderGrid({ 0, 0, 0 }, scale, quadsPerSide, { 1.f, 1.f, 1.f }, m_player.GetCamera().getViewProjectionMatrix(), id, m_heightmap.getBackingArray(), false);
  }

  void OnImGuiRender() override
  {
    if (ImGui::SliderInt("Width", &w, 10, 2000) + ImGui::SliderInt("Height", &h, 10, 2000) +
        ImGui::SliderFloat("Scale", &scale, 0, 50) + ImGui::SliderInt("Number of octaves", &o, 0, 10) +
        ImGui::SliderFloat("persistence", &p, 0, 1) + ImGui::SliderFloat("lacunarity", &l, 0, 50) +
        ImGui::SliderInt("seed", &seed, 0, 5)) {

      unsigned int nid;

      float *noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);
      m_heightmap.setHeights(w, h, noiseMap);

      // Texture stuff

      (glGenTextures(1, &nid));
      (glBindTexture(GL_TEXTURE_2D, nid));

      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

      if (noiseMap) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_FLOAT, noiseMap);
        glBindTexture(GL_TEXTURE_2D, 0);
      }

      id = nid;
    }

    ImGui::Checkbox("Fly", &m_playerIsFlying);
  }
};