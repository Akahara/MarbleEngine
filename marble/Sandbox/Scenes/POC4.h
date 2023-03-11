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

// FIX shadows in the mesa POC

class POC4Scene : public Scene {
private:
  Player                m_player;
  Renderer::TerrainMesh m_terrain;
  World::Sky            m_sky;
  float                 m_realTime;

  Renderer::FrameBufferObject m_depthFBO;
  std::shared_ptr<Renderer::Texture> m_depthTexture;

  struct Sun {
    Renderer::Camera camera;
    SunCameraHelper shadowCameraHelper;
  } m_sun;

public:
  POC4Scene()
  {
    using Renderer::Texture;

    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();

    auto &meshShader = Renderer::rebuildStandardMeshShader(Renderer::ShaderFactory()
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
    meshShader->bind();
    meshShader->setUniform1iv("u_Textures2D", 8, samplers);
    meshShader->setUniform1f("u_Strength", 1.25f);
    meshShader->setUniform3f("u_fogDamping", .005f, .005f, .007f);
    meshShader->setUniform3f("u_fogColor", 1.000f, 0.944f, 0.102f);
    Renderer::Shader::unbind();
    m_depthTexture = std::unique_ptr<Texture>(new Texture(Texture::createDepthTexture(1600 * 16 / 9, 1600)));
    m_depthFBO.setDepthTexture(*m_depthTexture);
    auto terrainMaterial = std::make_shared<Renderer::Material>();
    terrainMaterial->shader = meshShader;
    terrainMaterial->textures[0] = std::make_shared<Texture>("res/textures/sand.jpg");
    terrainMaterial->textures[5] = m_depthTexture;
    m_terrain.setMaterial(terrainMaterial);

    { // terrain
      constexpr float height = 10;
      constexpr unsigned int noiseMapSize = 200;

      Noise::PerlinNoiseSettings perlinSettings{};
      perlinSettings.scale = 30;
      perlinSettings.octaves = 2;
      perlinSettings.persistence = .5f;
      perlinSettings.initialFrequency = .5f;
      perlinSettings.lacunarity = 2.1f;
      perlinSettings.seed = 0;
      perlinSettings.terrainHeight = 1;
      Noise::ConcreteHeightMap heightmap = Noise::generateNoiseMap(noiseMapSize, noiseMapSize, perlinSettings);
      //Noise::outlineNoiseMap(&heightmap, -5, 2);
      for (int i = 0; i < (int)noiseMapSize; i++) {
        for (int j = 0; j < (int)noiseMapSize; j++) {
          float h = heightmap.getHeight(i, j);
          if (h > .6f) {
            h = Mathf::inverseLerp(.6f, .9f, h);
            h = (2/3.f + Mathf::smoothFloorLate(h*3)/2/3) * height + h * height/2.f;
          } else {
            h = height * .25f * Mathf::inverseLerp(0, .6f, h);
          }
          heightmap.setHeightAt(i, j, h);
        }
      }
      m_terrain.rebuildMesh(heightmap, { 0,0, noiseMapSize,noiseMapSize });
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
    for (const auto &chunk : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.worldBoundingBox;
      if (visibleFrustum.isOnFrustum(chunkAABB))
        m_sun.shadowCameraHelper.ensureCanReceiveShadows(chunkAABB);
    }

    m_sun.shadowCameraHelper.prepareSunCameraCasting();
    for (const auto &chunk : m_terrain.getChunks()) {
      const AABB &chunkAABB = chunk.worldBoundingBox;
      m_sun.shadowCameraHelper.ensureCanCastShadows(chunkAABB);
    }

    m_sun.shadowCameraHelper.finishSunCameraMovement();
    m_sun.camera = m_sun.shadowCameraHelper.getCamera();
  }

  void renderSceneDepthPass()
  {
    Renderer::Camera &camera = m_sun.camera;
    Renderer::clear();

    Renderer::renderMeshTerrain(camera, m_terrain);
  }

  void renderScene()
  {
    Renderer::Camera &camera = m_player.getCamera();
    Renderer::clear();

    Renderer::renderMeshTerrain(camera, m_terrain);

    m_sky.render(camera, m_realTime);
  }

  void onRender() override
  {
    Renderer::Camera &playerCamera = m_player.getCamera();
    Renderer::Frustum cameraFrustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(playerCamera);
    repositionSunCamera(cameraFrustum);

    auto &meshShader = Renderer::getStandardMeshShader();
    meshShader->bind();
    meshShader->setUniform3f("u_SunPos", m_sun.camera.getPosition());
    meshShader->setUniformMat4x3f("u_shadowMapProj", m_sun.shadowCameraHelper.getWorldToShadowMapProjectionMatrix());
    meshShader->setUniform2f("u_shadowMapOrthoZRange", m_sun.shadowCameraHelper.getZNear(), m_sun.shadowCameraHelper.getZFar());
    meshShader->setUniform1i("u_shadowMap", 5);
    Renderer::Shader::unbind();

    Renderer::beginDepthPass();
    m_depthFBO.bind();
    m_depthFBO.setViewportToTexture(*m_depthTexture);
    renderSceneDepthPass();

    Renderer::beginColorPass();
    Renderer::FrameBufferObject::unbind();
    Renderer::FrameBufferObject::setViewportToWindow();
    renderScene();
  }

  void onImGuiRender() override
  {
  }

  CAMERA_IS_PLAYER(m_player);
};