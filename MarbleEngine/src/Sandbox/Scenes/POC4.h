#pragma once

#include "../Scene.h"
#include "../../World/Sky.h"
#include "../../World/Grass.h"
#include "../../World/SunCameraHelper.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"

/* ========  A mesa scene with shadows and custom terrain shader (custom terrain showcase)  ======== */

class POC4Scene : public Scene {
private:
  Player              m_player;
  Terrain::Terrain    m_terrain;
  Renderer::Texture   m_sandTexture = Renderer::Texture("res/textures/sand.jpg");
  World::Sky          m_sky;
  float               m_realTime;

  Renderer::FrameBufferObject m_depthFBO;
  Renderer::Texture   m_depthTexture;

  struct Sun {
    Renderer::Camera camera;
    SunCameraHelper shadowCameraHelper;
  } m_sun;

public:
  POC4Scene()
  {
    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();

    Renderer::Shader &meshShader = Renderer::rebuildStandardMeshShader(Renderer::ShaderFactory()
      .prefix("res/shaders/")
      .addFileVertex("standard.vs")
      .prefix("mesh_parts/")
      .addFileFragment("base.fs")
      .addFileFragment("color_mesa.fs")
      .addFileFragment("lights_none.fs")
      .addFileFragment("final_fog.fs")
      .addFileFragment("shadows_casted.fs")
      .addFileFragment("normal_none.fs"));
    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    meshShader.bind();
    meshShader.setUniform1iv("u_Textures2D", 8, samplers);
    meshShader.setUniform1f("u_Strength", 1.25f);
    meshShader.setUniform3f("u_fogDamping", .005f, .005f, .007f);
    meshShader.setUniform3f("u_fogColor", 1.000f, 0.944f, 0.102f);
    Renderer::Shader::unbind();

    m_depthTexture = Renderer::Texture::createDepthTexture(1600 * 16 / 9, 1600);
    m_depthFBO.setDepthTexture(m_depthTexture);

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
      Noise::outlineNoiseMap(noiseMap, noiseMapSize, noiseMapSize, -5, 2);
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

  void repositionSunCamera(const Renderer::Frustum &visibleFrustum)
  {
    m_sun.shadowCameraHelper.setSunDirection({ glm::cos(m_realTime/10.f), .5f, glm::sin(m_realTime/10.f) });
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
    m_sandTexture.bind(0);
    m_depthTexture.bind(5);
    Renderer::FrameBufferObject::unbind();
    Renderer::FrameBufferObject::setViewportToWindow();
    renderScene();
  }

  void onImGuiRender() override
  {
  }

  CAMERA_IS_PLAYER(m_player);
};