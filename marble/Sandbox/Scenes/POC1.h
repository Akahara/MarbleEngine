#pragma once

#include "../Scene.h"

#include <glm/gtc/type_ptr.hpp>

#include "../../World/Sky.h"
#include "../../World/SunCameraHelper.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"
#include "../../abstraction/pipeline/VFXPipeline.h"

/* ========  A mountainy scene with erosion, shadows and flares  ======== */

class POC1Scene : public Scene {
private:
  World::Sky        m_sky{World::Sky::SkyboxesType::SNOW};
  Player            m_player;
  bool              m_playerIsFlying = true;
  float             m_realTime = 0;
  Renderer::TerrainMesh m_terrain;

  visualEffects::VFXPipeline m_pipeline;

  Renderer::FrameBufferObject m_depthFBO;
  std::shared_ptr<Renderer::Texture> m_depthTexture;

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
    auto &meshShader = Renderer::rebuildStandardMeshShader(Renderer::ShaderFactory()
      .prefix("res/shaders/")
      .addFileVertex("standard.vs")
      .prefix("mesh_parts/")
      .addFileFragment("base.fs")
      .addFileFragment("color_terrain.fs")
      .addFileFragment("lights_none.fs")
      .addFileFragment("final_fog.fs")
      .addFileFragment("shadows_casted.fs")
      .addFileFragment("normal_none.fs"));
    meshShader->bind();
    meshShader->setUniform1iv("u_Textures2D", 8, samplers);
    meshShader->setUniform1f("u_Strength", m_sun.strength);
    meshShader->setUniform3f("u_fogDamping", .001f, .001f, .001f);
    meshShader->setUniform3f("u_fogColor", 1.f, 1.f, 1.f);
    meshShader->setUniform2f("u_grassSteepness", .79f, 1.f);
    Renderer::Shader::unbind();

    m_depthTexture = std::make_shared<Renderer::Texture>(Renderer::Texture::createDepthTexture(1600 * 16 / 9, 1600));
    m_depthFBO.setDepthTexture(*m_depthTexture);

    auto terrainMaterial = std::make_shared<Renderer::Material>();
    terrainMaterial->shader = meshShader;
    terrainMaterial->textures[0] = std::make_shared<Renderer::Texture>("res/textures/snow_c.jpg");
    terrainMaterial->textures[1] = std::make_shared<Renderer::Texture>("res/textures/rock.jpg");
    terrainMaterial->textures[2] = std::make_shared<Renderer::Texture>("res/textures/snow_n.jpg");
    terrainMaterial->textures[5] = m_depthTexture;
    m_terrain.setMaterial(terrainMaterial);

    generateTerrain();

    m_sun.shadowCameraHelper.setSunDirection({ 0.f, 1.f, 4.f });

    m_pipeline.registerEffect<visualEffects::LensMask>();
    m_pipeline.registerEffect<visualEffects::GammaCorrection>();
    m_pipeline.registerEffect<visualEffects::Bloom>();
    m_pipeline.sortPipeline();
  }

  void generateTerrain()
  {
    // simple terrain + erosion
    constexpr unsigned int worldSize = 512;
    Noise::PerlinNoiseSettings perlinSettings;
    perlinSettings.scale = 30;
    perlinSettings.octaves = 4;
    perlinSettings.persistence = .5f;
    perlinSettings.initialFrequency = 1.f;
    perlinSettings.lacunarity = 2.1f;
    perlinSettings.seed = 0;
    perlinSettings.terrainHeight = 40.f;
    Noise::ConcreteHeightMap heightmap = Noise::generateNoiseMap(worldSize, worldSize, perlinSettings);
    Noise::ErosionSettings erosionSettings{};
    Noise::erode(&heightmap, erosionSettings);
    m_terrain.rebuildMesh(heightmap, { 0,0, worldSize,worldSize });
  }

  void step(float delta) override
  {
    m_realTime += delta;
    m_player.step(delta);
    m_pipeline.setContextParam<glm::vec3>("sunPos", m_sun.camera.getPosition());
    m_pipeline.setContextParam<glm::vec3>("cameraPos", getCamera().getForward());
    m_pipeline.setContextParam<Renderer::Camera>("camera", getCamera());
  }

  void repositionSunCamera(const Renderer::Frustum &visibleFrustum)
  {
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

    m_pipeline.bind();
    renderScene();
    m_pipeline.unbind();

    m_pipeline.renderPipeline();
  }

  void onImGuiRender() override
  {
    m_pipeline.onImGuiRender();
  }

  CAMERA_IS_PLAYER(m_player);

};