#pragma once

#include "../Scene.h"
#include "../../World/Sky.h"
#include "../../World/Grass.h"
#include "../../World/Props/PropsManager.h"
#include "../../World/SunCameraHelper.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"
#include "../../abstraction/pipeline/VFXPipeline.h"

class POC3Scene : public Scene {
private:
  Player              m_player;
  Terrain::Terrain    m_terrain;
  Renderer::Texture   m_sandTexture = Renderer::Texture("res/textures/sand1.jpg");
  Renderer::Texture   m_sandTexture_normal = Renderer::Texture("res/textures/sand1_normal.jpg");
  std::shared_ptr<Renderer::Texture>   m_rockTexture = std::make_shared<Renderer::Texture>("res/textures/Rock_9_Base_Color.jpg");
  World::Sky          m_sky{World::Sky::SkyboxesType::SAND};
  float               m_realTime;

  World::LightRenderer m_light;

  //std::shared_ptr<Renderer::Mesh> m_rock = std::make_shared < Renderer::Mesh >(Renderer::loadMeshFromFile("res/meshes/Rock_9.obj"));
  std::shared_ptr<Renderer::Mesh> m_arch = std::make_shared < Renderer::Mesh >(Renderer::loadMeshFromFile("res/meshes/SmallArch_Obj.obj"));

  visualEffects::VFXPipeline m_pipeline{ Window::getWinWidth(), Window::getWinHeight() };
  glm::vec3 m_sun{ 1000,1000,1000 };


  World::PropsManager m_props;

  bool rendersky = true;

  World::Water m_water;
  struct WaterData {
      float level = 9.2f;
      glm::vec2 position{ 80,80 };
      float size = 160.f;
  } m_waterData;

  int normalslot = 1;

  // TODO improve the desert scene
  // - another skybox DONE
  // - tweak the fog color/for damping
  // - cleaner texture DONE

public:
  POC3Scene()
  {
    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();

    // Uniforms stuff
    {
      int samplers[8] = { 0,1,2,3,4,5,6,7 };
      int normals_samplers[8] = { normalslot,-1,-1,-1,-1,-1,-1,-1};
      Renderer::Shader &meshShader = Renderer::getStandardMeshShader();
      meshShader.bind();
      meshShader.setUniform1iv("u_NormalsTextureSlot", 8, normals_samplers);
      meshShader.setUniform1iv("u_Textures2D", 8, samplers);
      meshShader.setUniform1i("u_castShadows", 0);
      meshShader.setUniform1i("u_RenderChunks", 0);
      meshShader.setUniform1f("u_Strength", 1.25f);
      meshShader.setUniform3f("u_fogDamping", .005f, .005f, .007f);
      meshShader.setUniform3f("u_fogColor", 1.000f, 0.944f, 0.102f);
      meshShader.setUniform2f("u_grassSteepness", 2.f, 2.2f); // disable grass

      Renderer::Shader::unbind();
    }

      m_props.feed(m_arch, glm::vec3{ 65,26,40 }, glm::vec3{ 0.5 });
     // m_props.feed(m_rock, glm::vec3{ 65,26,40 }, glm::vec3{ 0.05 });

    // VFX stuff
    {
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

    // Terrain Stuff
    { 
      constexpr unsigned int chunkSize = 20, chunkCount = 20;
      constexpr float height = 15;
      unsigned int noiseMapSize = 3 + chunkSize * chunkCount;
      float *noiseMap = Noise::generateNoiseMap(noiseMapSize,
                                                noiseMapSize,
                                                /*scale*/30,
                                                /*octaves*/2,
                                                /*persistence*/.5f,
                                                /*frequency*/.3f,
                                                /*lacunarity*/2.1f,
                                                /*seed*/0);
      Noise::rescaleNoiseMap(noiseMap, noiseMapSize, noiseMapSize, 0, 1, 0, 2*height);
      Terrain::ConcreteHeightMap *heightMap = new Terrain::ConcreteHeightMap(noiseMapSize, noiseMapSize, noiseMap);
      // apply h->H-|2h-H| to get that "desert feel"
      for (int i = 0; i < (int)noiseMapSize; i++) {
        for (int j = 0; j < (int)noiseMapSize; j++) {
          heightMap->setHeightAt(i, j, height - glm::abs(height - heightMap->getHeight(i, j)));
        }
      }
      m_terrain = Terrain::generateTerrain(heightMap, chunkCount, chunkCount, chunkSize);
    }

    // Water stuff
    m_water.addSource();
  }

  void step(float realDelta) override
  {
    m_player.step(realDelta);
    glm::vec3 playerPos = m_player.getPosition();
    if (m_terrain.isInSamplableRegion(playerPos.x, playerPos.z))
      playerPos.y = m_terrain.getHeight(playerPos.x, playerPos.z) + 3;
    m_player.setPostion(playerPos);
    m_player.updateCamera();
    m_realTime += realDelta;
    m_water.updateMoveFactor(realDelta);
  }

  void renderScene() 
  {
    Renderer::clear();

    Renderer::Camera& camera = m_player.getCamera();
    Renderer::Frustum cameraFrustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(camera);
    
    m_sandTexture.bind(0);
    m_sandTexture_normal.bind(1);
    for (const auto& [position, chunk] : m_terrain.getChunks()) {
      const AABB& chunkAABB = chunk.getMesh().getBoundingBox();

      if (!cameraFrustum.isOnFrustum(chunkAABB))
        continue;

      Renderer::renderMesh(camera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());
    }

    m_sandTexture.unbind();
    m_sandTexture_normal.unbind();
    
    m_props.render(camera);
    
    if (rendersky) m_sky.render(camera, m_realTime, false);
    
  }

  void onRender() override
  {
      
    m_pipeline.setContextParam<glm::vec3>("sunPos", m_sun);
    m_pipeline.setContextParam<glm::vec3>("cameraPos", getCamera().getForward());
    m_pipeline.setContextParam<Renderer::Camera>("camera", getCamera());
    m_pipeline.bind();

    m_water.onRender([this]() -> void { renderScene(); }, getCamera());
    //renderScene();
    m_pipeline.unbind();
    m_pipeline.renderPipeline();
      
  }

  void onImGuiRender() override
  {

      if (ImGui::DragFloat("WaterLevel", &m_waterData.level, 0.5f) +
          ImGui::DragFloat2("Water Position", &m_waterData.position.x, 1.f) +
          ImGui::DragFloat("WaterSize", &m_waterData.size, 1.f))
      {
          m_water.getSourceAt(0)->setHeight(m_waterData.level);
          m_water.getSourceAt(0)->setPosition(m_waterData.position);
          m_water.getSourceAt(0)->setSize(m_waterData.size);
      }
  
      ImGui::Checkbox("render sky", &rendersky);
    if (ImGui::Button("Turn on/off normal map")) {
        normalslot *= -1;
        int normals_samplers[8] = { normalslot,-1,-1,-1,-1,-1,-1,-1 };
        Renderer::Shader& meshShader = Renderer::getStandardMeshShader();
        meshShader.bind();
        meshShader.setUniform1iv("u_NormalsTextureSlot", 8, normals_samplers);
    }
    ImGui::Text("%d", normalslot);

    m_pipeline.onImGuiRender();
    m_light.onImguiRender();
    m_props.onImGuiRender();

    
  }

  CAMERA_IS_PLAYER(m_player);

};