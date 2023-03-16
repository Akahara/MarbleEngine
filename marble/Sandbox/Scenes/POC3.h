#pragma once

#include "../Scene.h"
#include "../../World/Sky.h"
#include "../../World/Grass.h"
#include "../../World/Props/PropsManager.h"
#include "../../World/SunCameraHelper.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"
#include "../../abstraction/pipeline/VFXPipeline.h"

/* ========  A desert scene with water  ======== */

class POC3Scene : public Scene {
private:
  Player               m_player;
  float                m_realTime;
  glm::vec3            m_sun{ 1000,1000,1000 };

  Noise::ConcreteHeightMap m_heightmap;
  Renderer::TerrainMesh    m_terrain;

  int m_normalSlot = 1; // set to -1 to deactivate

  visualEffects::VFXPipeline m_pipeline{ Window::getWinWidth(), Window::getWinHeight() };
  World::Sky           m_sky{World::Sky::SkyboxesType::SAND};
  World::LightRenderer m_light;
  World::PropsManager  m_props;
  World::Water         m_water;

public:
  POC3Scene()
  {
    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();

    { // Uniforms stuff
      int samplers[8] = { 0,1,2,3,4,5,6,7 };
      int normals_samplers[8] = { m_normalSlot,-1,-1,-1,-1,-1,-1,-1};
      auto &meshShader = Renderer::rebuildStandardMeshShader(Renderer::ShaderFactory()
        .prefix("res/shaders/")
        .addFileVertex("standard.vs")
        .prefix("mesh_parts/")
        .addFileFragment("base.fs")
        .addFileFragment("color_singletexture.fs")
        .addFileFragment("lights_pointlights.fs")
        .addFileFragment("final_fog.fs")
        .addFileFragment("shadows_normal.fs")
        .addFileFragment("normal_normalmap.fs"));
      meshShader->bind();
      meshShader->setUniform1iv("u_NormalsTextureSlot", 8, normals_samplers);
      meshShader->setUniform1iv("u_Textures2D", 8, samplers);
      meshShader->setUniform1f("u_Strength", 1.25f);
      meshShader->setUniform3f("u_fogDamping", .005f, .005f, .007f);
      meshShader->setUniform3f("u_fogColor", 1.000f, 0.944f, 0.102f);
      meshShader->setUniform3f("u_SunPos", 1000,1000,1000);
      Renderer::Shader::unbind();
    }

    { // props
      auto arch = std::make_shared<Renderer::Mesh>(Renderer::loadMeshFromFile("res/meshes/SmallArch_Obj.obj"));
      arch->getTransform().position = { 65,26,40 };
      arch->getTransform().scale = { .5f,.5f,.5f };
      m_props.feed(arch);
    }

    { // VFX stuff
      m_pipeline.registerEffect<visualEffects::LensMask>();
      m_pipeline.registerEffect<visualEffects::Bloom>();
      m_pipeline.registerEffect<visualEffects::Contrast>();
      m_pipeline.registerEffect<visualEffects::Saturation>();
      m_pipeline.registerEffect<visualEffects::Sharpness>();
      m_pipeline.registerEffect<visualEffects::GammaCorrection>();

      m_pipeline.sortPipeline();

      m_pipeline.addContextParam<glm::vec3>({ 1000,1000,1000 }, "sunPos");
      m_pipeline.addContextParam<glm::vec3>({ 10,10,10 }, "cameraPos");
      m_pipeline.addContextParam<Renderer::Camera>(getCamera(), "camera");
    }

    { // Terrain Stuff
      constexpr unsigned int worldSize = 400;
      Noise::PerlinNoiseSettings perlinSettings{};
      perlinSettings.scale = 30;
      perlinSettings.octaves = 2;
      perlinSettings.persistence = .5f;
      perlinSettings.initialFrequency = .5f;
      perlinSettings.lacunarity = 2.1f;
      perlinSettings.seed = 0;
      perlinSettings.terrainHeight = 15;
      m_heightmap = Noise::generateNoiseMap(worldSize, worldSize, perlinSettings);
      // apply h->H-|2h-H| to get that "desert feel"
      for (int i = 0; i < (int)worldSize; i++) {
        for (int j = 0; j < (int)worldSize; j++) {
          m_heightmap.setHeightAt(i, j, perlinSettings.terrainHeight - glm::abs(perlinSettings.terrainHeight - m_heightmap.getHeight(i, j)));
        }
      }
      m_terrain.rebuildMesh(m_heightmap, { 0,0, worldSize,worldSize });

      auto material = std::make_shared<Renderer::Material>();
      material->shader = Renderer::getStandardMeshShader();
      material->textures[0] = std::make_shared<Renderer::Texture>("res/textures/sand1.jpg");
      material->textures[1] = std::make_shared<Renderer::Texture>("res/textures/sand1_normal.jpg");
      m_terrain.setMaterial(material);
    }

    // Water stuff
    m_water.addSource({ 80,9.2f,80 }, { 160,160 });
  }

  void step(float realDelta) override
  {
    m_player.step(realDelta);
    glm::vec3 playerPos = m_player.getPosition();
    if (m_heightmap.isInBounds(playerPos.x, playerPos.z))
      playerPos.y = m_heightmap(playerPos.x, playerPos.z) + 3;
    m_player.setPostion(playerPos);
    m_player.updateCamera();
    m_realTime += realDelta;
    m_water.updateMoveFactor(realDelta);
  }

  void renderScene() 
  {
    Renderer::Camera& camera = m_player.getCamera();
    Renderer::clear();
    
    Renderer::renderMeshTerrain(camera, m_terrain);
    m_props.render(camera);
    m_sky.render(camera, m_realTime);
  }

  void onRender() override
  {
    m_pipeline.setContextParam<glm::vec3>("sunPos", m_sun);
    m_pipeline.setContextParam<glm::vec3>("cameraPos", getCamera().getForward());
    m_pipeline.setContextParam<Renderer::Camera>("camera", getCamera());
    m_pipeline.bind();

    m_water.onRender([this]() -> void { renderScene(); }, getCamera());
    m_pipeline.unbind();
    m_pipeline.renderPipeline();
  }

  void onImGuiRender() override
  {
    ImGui::DragFloat("Water position", glm::value_ptr(m_water.getSourceAt(0).position), 0.5f);
    ImGui::DragFloat("WaterSize", glm::value_ptr(m_water.getSourceAt(0).size), 1.f);

    if (ImGui::Button("Turn on/off normal map")) {
      m_normalSlot *= -1;
      int normals_samplers[8] = { m_normalSlot,-1,-1,-1,-1,-1,-1,-1 };
      auto& meshShader = Renderer::getStandardMeshShader();
      meshShader->bind();
      meshShader->setUniform1iv("u_NormalsTextureSlot", 8, normals_samplers);
      Renderer::Shader::unbind();
    }
    ImGui::Text("%d", m_normalSlot);

    m_pipeline.onImGuiRender();
    m_light.onImguiRender();
    m_props.onImGuiRender();
  }

  CAMERA_IS_PLAYER(m_player);

};