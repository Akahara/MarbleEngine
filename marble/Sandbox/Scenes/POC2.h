#pragma once

#include "../Scene.h"
#include "../../World/Sky.h"
#include "../../World/Grass.h"
#include "../../World/SunCameraHelper.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../World/Light/Light.h"
#include "../../World/Light/LightManager.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"

#include <queue>

/* ========  A flat scene with trees and grass  ======== */

class POC2Scene : public Scene {
private:
  struct CHUNKS {
    static constexpr unsigned int SIZE = 20;
    static constexpr unsigned int COUNT = 15;
  };
  struct GRASS_CHUNKS {
    static constexpr unsigned int LD_REGION = 1;
    static constexpr unsigned int HD_REGION = 2;
  };

  Player              m_player;
  World::Sky          m_sky;
  World::Water        m_water;
  float               m_realTime;

  Noise::ConcreteHeightMap m_heightmap;
  Renderer::TerrainMesh    m_terrain;
  World::TerrainGrass      m_grass;
  Renderer::InstancedMesh  m_smallTrees;
  Renderer::Mesh           m_lowPolyFatTreeMesh = Renderer::loadMeshFromFile("res/meshes/fattree.obj");

  World::LightRenderer m_light; // holds 12 lights, todo : change this

public:
  POC2Scene()
  {
    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    auto &meshShader = Renderer::rebuildStandardMeshShader(Renderer::ShaderFactory()
      .prefix("res/shaders/")
      .addFileVertex("standard.vs")
      .prefix("mesh_parts/")
      .addFileFragment("base.fs")
      .addFileFragment("color_terrain.fs")
      .addFileFragment("lights_none.fs")
      .addFileFragment("final_none.fs")
      .addFileFragment("shadows_normal.fs")
      .addFileFragment("normal_none.fs"));
    auto meshInstancedShader = Renderer::ShaderFactory()
      .prefix("res/shaders/")
      .addFileVertex("shadows_instanced.vs")
      .prefix("mesh_parts/")
      .addFileFragment("base.fs")
      .addFileFragment("color_terrain.fs")
      .addFileFragment("lights_none.fs")
      .addFileFragment("final_none.fs")
      .addFileFragment("shadows_normal.fs")
      .addFileFragment("normal_none.fs")
      .build();
    meshShader->bind();
    meshShader->setUniform1iv("u_Textures2D", 8, samplers);
    meshShader->setUniform1f("u_Strength", 1.25f);
    meshShader->setUniform2f("u_grassSteepness", .79f, 1.f);
    Renderer::Shader::unbind();

    { // terrain
      constexpr unsigned int worldSize = 3 + CHUNKS::SIZE * CHUNKS::COUNT;
      Noise::PerlinNoiseSettings perlinSettings;
      perlinSettings.scale = 30;
      perlinSettings.octaves = 4;
      perlinSettings.persistence = .5f;
      perlinSettings.initialFrequency = 1.f;
      perlinSettings.lacunarity = 2.1f;
      perlinSettings.seed = 0;
      perlinSettings.terrainHeight = 5.f;
      m_heightmap = Noise::generateNoiseMap(worldSize, worldSize, perlinSettings);
      // make the terrain plunge toward the middle, so that more can be seen from far away
      for (int i = 0; i < (int)worldSize; i++) {
        for (int j = 0; j < (int)worldSize; j++) {
          float dx = glm::abs(i - worldSize / 2.f);
          float dy = glm::abs(j - worldSize / 2.f);
          m_heightmap.setHeightAt(i, j, m_heightmap.getHeight(i, j) + glm::max(dx, dy) / (worldSize / 2.f) * 10.f);
        }
      }
      m_terrain.rebuildMesh(m_heightmap, { 0,0, worldSize,worldSize });
      auto terrainMaterial = std::make_shared<Renderer::Material>();
      terrainMaterial->shader = meshShader;
      terrainMaterial->textures[0] = terrainMaterial->textures[1] = std::make_shared<Renderer::Texture>("res/textures/black.png");
      m_terrain.setMaterial(terrainMaterial);
    }

    { // grass
      std::vector<glm::ivec2> hdChunks, ldChunks;

      constexpr unsigned int ldRegion = GRASS_CHUNKS::LD_REGION;
      constexpr unsigned int grassChunksCount = ldRegion * 2 + GRASS_CHUNKS::HD_REGION;
      constexpr unsigned int grassSIZE = CHUNKS::SIZE * CHUNKS::COUNT / grassChunksCount;
      for (unsigned int x = 0; x < grassChunksCount; x++) {
        for (unsigned int y = 0; y < grassChunksCount; y++) {
          if (x < ldRegion || y < ldRegion || x >= grassChunksCount - ldRegion || y >= grassChunksCount - ldRegion)
            ldChunks.push_back({ x,y });
          else
            hdChunks.push_back({ x,y });
        }
      }

      m_grass = World::TerrainGrass(std::make_unique<World::FixedGrassChunks>(
        std::make_unique<World::TerrainGrassGenerator>(&m_heightmap),
        grassSIZE,
        hdChunks,
        ldChunks
      ));
    }

    { // small trees
      auto treeMesh = Renderer::loadMeshFromFile("res/meshes/lowtree.obj");
      std::vector<Renderer::BaseInstance> trees;

      for (int i = 0; i < 20; i++) {
        float x, z;
        x = (float)(rand() % (3 + CHUNKS::SIZE * CHUNKS::COUNT));
        z = (float)(rand() % (3 + CHUNKS::SIZE * CHUNKS::COUNT));

        float size = (float)(rand() % 10 + 4);

        Renderer::BaseInstance &instance = trees.emplace_back();
        instance.position = { x, m_heightmap(x, z) - 2.F, z };
        instance.scale = glm::vec3(size);
      }

      m_smallTrees = Renderer::InstancedMesh(treeMesh.getModel(), treeMesh.getMaterial(), trees.size(), trees.data());
      m_smallTrees.getMaterial()->shader = meshInstancedShader;
    }

    { // big tree
      m_lowPolyFatTreeMesh.getTransform().position = { 150, m_heightmap.getHeight(150,150), 150 };
      m_lowPolyFatTreeMesh.getTransform().scale = glm::vec3(4);
    }
  }

  void step(float realDelta) override
  {
    m_player.step(realDelta);
    glm::vec3 playerPos = m_player.getPosition();
    if(m_heightmap.isInBounds(playerPos.x, playerPos.z))
      playerPos.y = m_heightmap(playerPos.x, playerPos.z) + 3;
    m_player.setPostion(playerPos);
    m_player.updateCamera();
    m_grass.step(m_player.getCamera());
    m_realTime += realDelta;
  }

  void onRender() override
  {
    Renderer::clear();

    Renderer::Camera &camera = m_player.getCamera();
    Renderer::Frustum cameraFrustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(camera);

    Renderer::renderMeshTerrain(camera, m_terrain);
    Renderer::renderMeshInstanced(camera, m_smallTrees);
    if (cameraFrustum.isOnFrustum(m_lowPolyFatTreeMesh.getBoundingBox()))
      Renderer::renderMesh(camera, m_lowPolyFatTreeMesh);

    m_grass.render(camera, m_realTime);
    m_sky.render(camera, m_realTime);
  }

  void onImGuiRender() override
  {
      glm::vec3 pos = m_player.getPosition();
      ImGui::Text("x : %f | y : %f | z : %f\n", pos.x, pos.y, pos.z);
      m_light.onImguiRender();
  }

  CAMERA_IS_PLAYER(m_player);

};