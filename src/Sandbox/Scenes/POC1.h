#pragma once

#include "../Scene.h"
#include "../../World/Sky.h"
#include "../../World/SunCameraHelper.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"

class POC1Scene : public Scene {
private:
  World::Sky        m_sky;
  Player            m_player;
  bool              m_playerIsFlying = true;
  float             m_realTime = 0;
  Terrain::Terrain  m_terrain;

  Renderer::Texture m_rockTexture = Renderer::Texture("res/textures/rock.jpg");
  Renderer::Texture m_grassTexture = Renderer::Texture("res/textures/grass6.jpg");

  Renderer::FrameBufferObject m_depthFBO;
  Renderer::Texture m_depthTexture;

  struct Sun {
    Renderer::Camera camera;
    float strength = 1.25f;
    SunCameraHelper shadowCameraHelper;
  } m_sun;

public:
  POC1Scene()
  {
    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();
    m_sun.camera.setPosition({ 1,1,1 });
    m_sun.camera.recalculateViewProjectionMatrix();

    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::Shader &meshShader = Renderer::getStandardMeshShader();
    meshShader.bind();
    meshShader.setUniform1iv("u_Textures2D", 8, samplers);
    meshShader.setUniform1i("u_castShadows", 1);
    meshShader.setUniform1i("u_RenderChunks", 0);
    meshShader.setUniform1f("u_Strength", m_sun.strength);
    meshShader.setUniform3f("u_fogDamping", .003f, .005f, .007f);
    meshShader.setUniform3f("u_fogColor", .71f, .86f, 1.f);
    meshShader.setUniform2f("u_grassSteepness", .79f, 1.f);
    Renderer::Shader::unbind();

    m_depthTexture = Renderer::Texture::createDepthTexture(1600 * 16 / 9, 1600);
    m_depthFBO.setDepthTexture(m_depthTexture);

    generateTerrain();
  }

  void generateTerrain()
  {
    // 20x20 * 20x20 ~= 512x512 which is a good size of the erosion algorithm
    constexpr unsigned int chunkSize = 20;
    constexpr unsigned int chunkCount = 20;

    { // simple terrain + erosion
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
  }

  void step(float delta) override
  {
    m_realTime += delta;
    m_player.step(delta);
  }

  void repositionSunCamera(const Renderer::Frustum &visibleFrustum)
  {
    m_sun.shadowCameraHelper.setSunDirection({ 0.f, 1.f, 4.f });
    m_sun.shadowCameraHelper.prepareSunCameraMovement();
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();
      if (visibleFrustum.isOnFrustum(chunkAABB))
        m_sun.shadowCameraHelper.ensureCanReceiveShadows(chunkAABB);
    }

    m_sun.shadowCameraHelper.prepareSunCameraCasting();
    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();
      m_sun.shadowCameraHelper.ensureCanCastShadows(chunkAABB);
    }

    m_sun.shadowCameraHelper.finishSunCameraMovement();
    m_sun.camera = m_sun.shadowCameraHelper.getCamera();
  }

  void renderSceneDepthPass()
  {
    // TODO use an empty shader to make the shadow map
    Renderer::Camera &camera = m_sun.camera;
    Renderer::clear();

    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();

      if (!m_sun.shadowCameraHelper.isBoxVisibleBySun(chunkAABB))
        continue;

      Renderer::renderMesh(camera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());
    }
  }

  void renderScene()
  {
    Renderer::clear();

    Renderer::Camera &camera = m_player.getCamera();
    Renderer::Frustum cameraFrustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(camera);

    for (const auto &[position, chunk] : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.getMesh().getBoundingBox();

      if (!cameraFrustum.isOnFrustum(chunkAABB))
        continue;

      Renderer::renderMesh(camera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());
    }

    m_sky.render(camera, m_realTime);
  }

  void onRender() override
  {
    Renderer::Camera &playerCamera = m_player.getCamera();
    Renderer::Frustum cameraFrustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(playerCamera);
    repositionSunCamera(cameraFrustum);

    Renderer::Shader &meshShader = Renderer::getStandardMeshShader();
    meshShader.bind();
    meshShader.setUniform3f("u_SunPos", m_sun.camera.getPosition());
    meshShader.setUniformMat4x3f("u_shadowMapProj", m_sun.shadowCameraHelper.getWorldToShadowMapProjectionMatrix());
    meshShader.setUniform2f("u_shadowMapOrthoZRange", m_sun.shadowCameraHelper.getZNear(), m_sun.shadowCameraHelper.getZFar());
    meshShader.setUniform1i("u_shadowMap", 5);
    Renderer::getStandardMeshShader().unbind();

    Renderer::beginDepthPass();
    m_depthFBO.bind();
    m_depthFBO.setViewportToTexture(m_depthTexture);
    renderSceneDepthPass();

    Renderer::beginColorPass();
    m_rockTexture.bind(0);
    m_grassTexture.bind(1);
    m_depthTexture.bind(5);
    Renderer::FrameBufferObject::unbind();
    Renderer::FrameBufferObject::setViewportToWindow();
    renderScene();
  }

  void onImGuiRender() override
  {
  }
};