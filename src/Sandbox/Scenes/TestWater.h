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

#include "../../World/Water/WaterSource.h"
#include "../../World/Water/WaterRenderer.h"


#include "../../Utils/AABB.h"
#include "../Scenes/TestShadows.h"

class TestWater : public Scene {


private:

    //=====================================================================================================================//

      /* Basic scene stuff */
    Renderer::Cubemap m_skybox;
    Player            m_player;
    bool              m_playerIsFlying = true;
    float             realTime = 0;
    Player            m_roguePlayer; // another player used to better visualize frustum culling
    bool              m_isRoguePlayerActive = false;

    /* Terrain generation stuff */
    Renderer::Mesh                    m_terrainMesh;
    TerrainMeshGenerator::Terrain     m_terrain;      // holds heightmap and chunksize
    TerrainMeshGenerator::TerrainData m_terrainData;  // < This holds default and nice configuration for the terrain
    unsigned int                      m_terrainWidthInChunks = 10, m_terrainHeightInChunks = 10;
    int                               m_chunkSize = 16;

    /*  Water Stuff     */    
    WaterRenderer m_waterRenderer;
    WaterSource m_waterSource{ 0, {0,0} };

    /* Rendering stuff */
    Renderer::Frustum     m_frustum;

    Renderer::Texture     m_rockTexture = Renderer::Texture("res/textures/rock.jpg");
    Renderer::Texture     m_grassTexture = Renderer::Texture("res/textures/grass6.jpg");

    //=====================================================================================================================//

public:
    TestWater()
        : m_skybox{
          "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
          "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
          "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
    {
        m_rockTexture.bind(0);
        m_grassTexture.bind(1);

        m_player.setPostion({ 100.f, 22.F , 100.f });
        m_player.updateCamera();

        int samplers[8] = { 0,1,2,3,4,5,6,7 };
        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);


        m_terrain = TerrainMeshGenerator::generateTerrain(
            m_terrainData,
            m_terrainWidthInChunks,
            m_terrainHeightInChunks,
            m_chunkSize);

        m_frustum = Renderer::Frustum::createFrustumFromCamera(m_player.getCamera() );

    }

    void step(float delta) override
    {
        realTime += delta;
        m_player.step(delta);

        m_frustum = Renderer::Frustum::createFrustumFromCamera(m_player.getCamera());
    }

    void onRender() override
    {
        // Render scene
        {

        Renderer::Renderer::clear();
        Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());

        m_rockTexture.bind(0);
        m_grassTexture.bind(1);

        for (const auto& [position, chunk] : m_terrain.getChunks()) {

            const AABB& chunkAABB = chunk.getMesh().getBoundingBox();

            bool isVisible = Renderer::Frustum::isOnFrustum(m_frustum, chunkAABB);

            if (isVisible) {
                Renderer::renderMesh(glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh(), m_player.getCamera());
            }
        }
        }


    }

    void onImGuiRender() override
    {
    }
};