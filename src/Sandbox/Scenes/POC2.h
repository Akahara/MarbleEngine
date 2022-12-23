#pragma once

#include "../Scene.h"
#include "../../World/Sky.h"
#include "../../World/Grass.h"
#include "../../World/SunCameraHelper.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"

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
  World::TerrainGrass m_grass;
  Terrain::Terrain    m_terrain;
  World::Sky          m_sky;
  World::Water      m_water;
  float               m_realTime;


  Renderer::Texture     m_grassTexture = Renderer::Texture("res/textures/grass6.jpg");

  Renderer::Mesh      m_lowPolyTreeMesh = Renderer::loadMeshFromFile("res/meshes/lowtree.obj");

  struct Tree {

      glm::vec3 position;
      glm::vec3 size;
      Renderer::Mesh* mesh;

      void render(const Renderer::Camera& camera) {
          Renderer::renderMesh(camera, position, size, *mesh);
      }

  };
  std::vector<Tree> m_trees;

public:
  POC2Scene()
  {
    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::Shader &meshShader = Renderer::getStandardMeshShader();
    meshShader.bind();
    meshShader.setUniform1iv("u_Textures2D", 8, samplers);
    meshShader.setUniform1i("u_castShadows", 0);
    meshShader.setUniform1i("u_RenderChunks", 0);
    meshShader.setUniform1f("u_Strength", 1.25f);
    meshShader.setUniform3f("u_fogDamping", .003f, .005f, .007f);
    meshShader.setUniform3f("u_fogColor", .71f, .86f, 1.f);
    meshShader.setUniform2f("u_grassSteepness", .79f, 1.f);
    Renderer::Shader::unbind();

    { // terrain
      unsigned int noiseMapSize = 3 + CHUNKS::SIZE * CHUNKS::COUNT;
      float *noiseMap = Noise::generateNoiseMap(noiseMapSize,
                                                noiseMapSize,
                                                /*scale*/30,
                                                /*octaves*/4,
                                                /*persistence*/.5f,
                                                /*frequency*/1.f,
                                                /*lacunarity*/2.1f,
                                                /*seed*/0);
      Noise::rescaleNoiseMap(noiseMap, noiseMapSize, noiseMapSize, 0, 1, 0, /*height*/5);
      Terrain::ConcreteHeightMap *heightMap = new Terrain::ConcreteHeightMap(noiseMapSize, noiseMapSize, noiseMap);
      // make the terrain plunge toward the middle, so that more can be seen from far away
      for (int i = 0; i < (int)noiseMapSize; i++) {
        for (int j = 0; j < (int)noiseMapSize; j++) {
          float dx = glm::abs(i - noiseMapSize / 2.f);
          float dy = glm::abs(j - noiseMapSize / 2.f);
          heightMap->setHeightAt(i, j, heightMap->getHeight(i, j) + glm::max(dx, dy)/(noiseMapSize/2.f)*10.f);
        }
      }
      m_terrain = Terrain::generateTerrain(heightMap, CHUNKS::COUNT, CHUNKS::COUNT, CHUNKS::SIZE);
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
        std::make_unique<World::TerrainGrassGenerator>(&m_terrain),
        grassSIZE,
        hdChunks,
        ldChunks
      ));
    }


    for (int i = 0; i < 20; i++) {


        float x, z;
        x = rand() % (3 + CHUNKS::SIZE * CHUNKS::COUNT);
        z = rand() %( 3 + CHUNKS::SIZE * CHUNKS::COUNT);




        if (m_terrain.isInSamplableRegion(x, z)) {

            float size = rand() % 10 + 4;

            Tree tree{

                 glm::vec3{ x, m_terrain.getHeight(x, z) - 2.F, z },
                glm::vec3(size),
                &m_lowPolyTreeMesh

            };

            m_trees.push_back(tree);

        }
    }


  }

  void step(float realDelta) override
  {
    m_player.step(realDelta);
    glm::vec3 playerPos = m_player.getPosition();
    if(m_terrain.isInSamplableRegion(playerPos.x, playerPos.z))
      playerPos.y = m_terrain.getHeight(playerPos.x, playerPos.z) + 3;
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

    // no textures are bound, the ground will be black
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();

      if (!cameraFrustum.isOnFrustum(chunkAABB))
        continue;

      Renderer::renderMesh(camera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());

      m_grassTexture.bind(0);
      
      for (auto& t : m_trees) {
          if (!cameraFrustum.isOnFrustum(m_lowPolyTreeMesh.getBoundingBoxInstance(t.position, t.size))) {
              continue;
          }
          t.render(camera);
      }
      


    }



    m_grass.render(camera, m_realTime);
    m_sky.render(camera, m_realTime);
  }

  void onImGuiRender() override
  {
      glm::vec3 pos = m_player.getPosition();
      ImGui::Text("x : %f | y : %f | z : %f\n", pos.x, pos.y, pos.z);
  }

  CAMERA_IS_PLAYER();

};