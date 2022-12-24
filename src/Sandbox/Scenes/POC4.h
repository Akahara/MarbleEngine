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

class POC4Scene : public Scene {
private:
  Player              m_player;
  Terrain::Terrain    m_terrain;
  Renderer::Texture   m_sandTexture = Renderer::Texture("res/textures/sand.jpg");
  World::Sky          m_sky;
  float               m_realTime;

public:
  POC4Scene()
  {
    std::cout << "Do not forget to add #define MESA_SCENE in the standard mesh shader!" << std::endl;

    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::Shader &meshShader = Renderer::getStandardMeshShader();
    meshShader.bind();
    meshShader.setUniform1iv("u_Textures2D", 8, samplers);
    meshShader.setUniform1i("u_castShadows", 0);
    meshShader.setUniform1i("u_RenderChunks", 0);
    meshShader.setUniform1f("u_Strength", 1.25f);
    meshShader.setUniform3f("u_fogDamping", .005f, .005f, .007f);
    meshShader.setUniform3f("u_fogColor", 1.000f, 0.944f, 0.102f);
    meshShader.setUniform2f("u_grassSteepness", 2.f, 2.2f); // disable grass
    Renderer::Shader::unbind();

    { // terrain
      constexpr unsigned int chunkSize = 20, chunkCount = 20;
      constexpr float height = 10;
      unsigned int noiseMapSize = 3 + chunkSize * chunkCount;
      float *noiseMap = Noise::generateNoiseMap(noiseMapSize,
                                                noiseMapSize,
                                                /*scale*/30,
                                                /*octaves*/2,
                                                /*persistence*/.5f,
                                                /*frequency*/.5f,
                                                /*lacunarity*/2.1f,
                                                /*seed*/0);
      Terrain::ConcreteHeightMap *heightMap = new Terrain::ConcreteHeightMap(noiseMapSize, noiseMapSize, noiseMap);
      for (int i = 0; i < (int)noiseMapSize; i++) {
        for (int j = 0; j < (int)noiseMapSize; j++) {
          float h = heightMap->getHeight(i, j);
          if (h > .6f) {
            h = Mathf::inverseLerp(.6f, .9f, h);
            h = (2/3.f + Mathf::smoothFloorLate(h*3)/2/3) * height + h * height/2.f;
          } else {
            h = height * .25f * Mathf::inverseLerp(0, .6f, h);
          }
          heightMap->setHeightAt(i, j, h);
        }
      }
      m_terrain = Terrain::generateTerrain(heightMap, chunkCount, chunkCount, chunkSize);
    }
  }
  
  void step(float realDelta) override
  {
    m_player.step(realDelta);
    m_player.updateCamera();
    m_realTime += realDelta;
  }

  void onRender() override
  {
    Renderer::clear();

    Renderer::Camera &camera = m_player.getCamera();
    Renderer::Frustum cameraFrustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(camera);

    m_sandTexture.bind(0);
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();

      if (!cameraFrustum.isOnFrustum(chunkAABB))
        continue;

      Renderer::renderMesh(camera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());
    }

    m_sky.render(camera, m_realTime, false);
  }

  void onImGuiRender() override
  {
  }

  CAMERA_IS_PLAYER(m_player);
};