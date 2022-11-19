#pragma once

#include "../Scene.h"
#include "../../World/Sky.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../abstraction/UnifiedRenderer.h"

class POC1Scene : public Scene {
private:
  World::Sky        m_sky;
  Player            m_player;
  bool              m_playerIsFlying = true;
  float             m_realTime = 0;
  Terrain::Terrain  m_terrain;

  Renderer::Texture m_rockTexture = Renderer::Texture("res/textures/rock.jpg");
  Renderer::Texture m_grassTexture = Renderer::Texture("res/textures/grass6.jpg");

  struct Sun {
    glm::vec3 position{ 0.f };
    float strength = 1.25f;
    SunCameraHelper shadowCameraHelper;
  } m_sun;

  //=====================================================================================================================//

public:
  POC1Scene()
  {
    m_rockTexture.bind(0);
    m_grassTexture.bind(1);

    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();
    m_sun.position = { 100,100,100 };

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);

    generateTerrain();
  }

  void generateTerrain()
  {
    // 20x20 * 20x20 ~= 512x512 which is a good size of the erosion algorithm
    constexpr unsigned int chunkSize = 20;
    constexpr unsigned int chunkCount = 20;

    // simple terrain + erosion
    unsigned int noiseMapSize = 3 + chunkSize * chunkCount;
    float *noiseMap = Noise::generateNoiseMap(noiseMapSize,
                                              noiseMapSize,
                                     /*scale*/30,
                                   /*octaves*/4,
                               /*persistence*/.5f,
                                /*lacunarity*/2.1f,
                                      /*seed*/0);
    Noise::ErosionSettings erosionSettings{};
    Noise::erode(noiseMap, noiseMapSize, erosionSettings);
    Noise::rescaleNoiseMap(noiseMap, noiseMapSize, noiseMapSize, 0, 1, 0, /*terrain height*/25.f);
    Terrain::HeightMap *heightMap = new Terrain::ConcreteHeightMap(noiseMapSize, noiseMapSize, noiseMap);
    m_terrain = Terrain::generateTerrain(heightMap, chunkCount, chunkCount, chunkSize);
  }

  void step(float delta) override
  {
    m_realTime += delta;
    m_player.step(delta);
  }

  void repositionSunCamera(const Renderer::Frustum &visibleFrustum)
  {
    m_sun.shadowCameraHelper.prepareSunCameraMovement();
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();
      if(visibleFrustum.isOnFrustum(chunkAABB))
        m_sun.shadowCameraHelper.ensureCanReceiveShadows(chunkAABB);
    }

    m_sun.shadowCameraHelper.prepareSunCameraCasting();
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();
      m_sun.shadowCameraHelper.ensureCanReceiveShadows(chunkAABB);
      m_sun.shadowCameraHelper.ensureCanCastShadows(chunkAABB);
    }

    m_sun.shadowCameraHelper.finishSunCameraMovement();
    // TODO finish integrating shadows in the POC 1
    //m_sun.position = m_sun.shadowCameraHelper.getCamera();
  }

  void onRender() override
  {
    Renderer::Camera &renderCamera = m_player.getCamera();
    Renderer::Frustum cameraFrustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(renderCamera);

    Renderer::clear();
    m_sky.render(renderCamera, m_realTime);

    Renderer::Shader &meshShader = Renderer::getStandardMeshShader();
    meshShader.bind();
    meshShader.setUniform3f("u_SunPos", m_sun.position.x, m_sun.position.y, m_sun.position.z);
    meshShader.setUniform1f("u_Strength", m_sun.strength);
    meshShader.setUniform1i("u_RenderChunks", 1);
    meshShader.setUniform3f("u_fogDamping", .003f, .01f, .013f);
    meshShader.setUniform3f("u_fogColor", .71, .86, 1.);
    meshShader.setUniform2f("u_grassSteepness", .79f, 1.f);
    meshShader.setUniform1i("u_RenderChunks", 0);
    Renderer::getStandardMeshShader().unbind();

    repositionSunCamera(cameraFrustum);

    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();

      if (!cameraFrustum.isOnFrustum(chunkAABB))
        continue;

      Renderer::renderMesh(renderCamera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());
    }
  }

  void onImGuiRender() override
  {
  }
};