#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Mesh.h"
#include "../../World/Player.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/MapUtilities.h"
#include "../../World/TerrainGeneration/Noise.h"

#include "../../Utils/AABB.h"
#include "../Scenes/TestShadows.h"



class TestTerrainScene : public Scene {


private:

  //=====================================================================================================================//

    /* Basic scene stuff */
  Renderer::Cubemap m_skybox;
  Player            m_player;
  bool              m_playerIsFlying = true;
  float             realTime = 0;

    /* Terrain generation stuff */
  Renderer::Mesh                    m_terrainMesh;
  TerrainMeshGenerator::Terrain     m_terrain;      // holds heightmap and chunksize
  TerrainMeshGenerator::TerrainData m_terrainData; // < This holds default and nice configuration for the terrain
  int                               m_numberOfChunks = 8;


    /* Rendering stuff */
  Renderer::Frustum     m_frustum;
  
  Renderer::Texture     m_rockTexture = Renderer::Texture( "res/textures/rock.jpg" );
  Renderer::Texture     m_grassTexture = Renderer::Texture( "res/textures/grass6.jpg" );

  float                 m_mSize = 1; // < Terrain mesh size
  bool                  m_renderChunks = 0;



  /* Other */

  float test = 1;

  Renderer::Mesh        m_treeMesh = Renderer::loadMeshFromFile("res/meshes/Tree.obj"); // <! CHANGER CE PUTAIN DE SOL
  Renderer::Mesh        m_grassBlade = Renderer::loadMeshFromFile("res/meshes/blade.obj"); 

  struct Sun {

      glm::vec3 position;
      float strength = 1.25f;

  } m_sun;

  //=====================================================================================================================//

public:
  TestTerrainScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
  {



      m_rockTexture.bind(0);
      m_grassTexture.bind(1);


    m_player.setPostion({ 100.f, 22.F , 100.f });
    m_player.updateCamera();
    regenerateTerrain();
    m_sun.position = { 100,100,100 };


    int samplers[8] = { 0,1,2,3,4,5,6,7 };
    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);

    Renderer::getStandardMeshShader().setUniform1f("u_Strenght", m_sun.strength);

    m_terrain = TerrainMeshGenerator::generateTerrain(m_terrainData, m_numberOfChunks);

    m_frustum = Renderer::Frustum::createFrustumFromCamera(
        m_player.getCamera(),
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().aspect,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().fovy,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zNear,
        m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zFar
    );



  }

  void regenerateTerrain()
  {

        free(m_terrainData.noiseMap);

        m_terrainData.noiseMap = Noise::generateNoiseMap(
            m_terrainData.width,
            m_terrainData.height,
            m_terrainData.scale,
            m_terrainData.octaves,
            m_terrainData.persistence,
            m_terrainData.lacunarity,
            m_terrainData.seed
        );


        m_terrain.heightMap.setHeights(
            m_terrainData.width,
            m_terrainData.height,
            m_terrainData.noiseMap
        );

        m_terrain = TerrainMeshGenerator::generateTerrain(m_terrainData, m_numberOfChunks);

  }

  void step(float delta) override
  {
      realTime += delta;
      m_player.step(delta);
      if (!m_playerIsFlying) {
          glm::vec3 pos = m_player.getPosition();
          pos.y = m_terrain.heightMap.getHeightLerp(pos.x, pos.z) + 1.f;
          m_player.setPostion(pos);
          m_player.updateCamera();
      }
    
      m_frustum = Renderer::Frustum::createFrustumFromCamera(
          m_player.getCamera(),
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().aspect,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().fovy,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zNear,
          m_player.getCamera().getProjection<Renderer::PerspectiveProjection>().zFar
      );
    

  }

  void onRender() override
  {
    Renderer::Renderer::clear();
    Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera(), m_player.getPosition());

    // TODO! : fix the texture issue in the standard mesh shader
    m_rockTexture.bind(0);
    m_grassTexture.bind(1);

    AABB aabbtemp;
    aabbtemp.setSize(glm::vec3(m_terrain.chunkSize));

    for (const auto& [position, chunk] : m_terrain.chunksPosition) {
            
           aabbtemp.setOrigin(glm::vec3{ position.x, 0.f, position.y } * m_mSize);

           if (Renderer::Frustum::isOnFrustum(m_frustum, aabbtemp)) {

               Renderer::renderMesh(glm::vec3{ position.x , 0.F, position.y} * m_mSize , glm::vec3(m_mSize), chunk.mesh, m_player.getCamera());
               if (DebugWindow::renderAABB()) (renderAABBDebugOutline(m_player.getCamera(), aabbtemp));
           }
    }
    Renderer::renderMesh(glm::vec3{ 100.f ,m_terrain.heightMap.getHeight(100.f, 100.f) * m_terrainData.terrainHeight, 100.F}, glm::vec3(2), m_treeMesh, m_player.getCamera(), true);
    /*
    for (unsigned int i = 0; i < 100; i++) {
        for (unsigned int j = 0; j < 100; j++) {

            if (Renderer::Frustum::isOnFrustum(m_frustum, m_grassBlade.getBoundingBox())) {
                Renderer::renderMesh(glm::vec3{ i ,m_terrain.heightMap.getHeight(i, j) * m_terrainData.terrainHeight, j }, glm::vec3(10), m_grassBlade, m_player.getCamera(), true);
            }

            Renderer::renderMesh(glm::vec3{ i ,m_terrain.heightMap.getHeight(i, j) * m_terrainData.terrainHeight, j }, glm::vec3(10), m_grassBlade, m_player.getCamera(), true);
        }
    }
    */

    renderAABBDebugOutline(m_player.getCamera(), m_treeMesh.getBoundingBox());



    Renderer::getStandardMeshShader().bind();
    Renderer::getStandardMeshShader().setUniform3f("u_SunPos", m_sun.position.x, m_sun.position.y, m_sun.position.z);
    Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 0);
    if (m_renderChunks)
        Renderer::getStandardMeshShader().setUniform1i("u_RenderChunks", 1);
    Renderer::getStandardMeshShader().unbind();

  }

  void onImGuiRender() override
  {
      
    if (ImGui::SliderInt("Width", (int*)&m_terrainData.width, 10, 2000) + ImGui::SliderInt("Height", (int*)& m_terrainData.height, 10, 2000) +
        ImGui::SliderFloat("Scale", &m_terrainData.scale, 0, 50) + ImGui::SliderInt("Number of octaves", &m_terrainData.octaves, 0, 10) +
        ImGui::SliderFloat("persistence", &m_terrainData.persistence, 0, 1) + ImGui::SliderFloat("lacunarity", &m_terrainData.lacunarity, 0, 50) +
        ImGui::SliderInt("seed", &m_terrainData.seed, 0, 5) + ImGui::SliderFloat("Depth", &m_terrainData.terrainHeight, 0, 100.f) +
        ImGui::SliderInt("chunksize", &m_numberOfChunks, 1, 16)) {
      regenerateTerrain();
    }

    if (ImGui::SliderFloat("test ", &test, 0, 1)) {
        m_treeMesh.getBoundingBox().setSize(glm::vec3(test));

    }
      
    ImGui::SliderFloat3("Sun position", &m_sun.position[0], -200, 200);
    ImGui::Text("X : %f , Y : %f, Z : %f", m_player.getPosition().x, m_player.getPosition().y, m_player.getPosition().z);
    ImGui::Checkbox("Fly", &m_playerIsFlying);
    ImGui::Checkbox("Render Chunks", &m_renderChunks);
  }
};