#pragma once

#include "../Scene.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Mesh.h"
#include "../../World/Player.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/Noise.h"

#include "../../World/Water/WaterSource.h"
#include "../../World/Water/WaterRenderer.h"
#include "../../World/Water/Water.h"


#include "../../Utils/AABB.h"
#include "../Scenes/TestShadows.h"

#include <vector>

class TestWater : public Scene {
private:

    //=====================================================================================================================//

      /* Basic scene stuff */
    Player            m_player;
    bool              m_playerIsFlying = true;

    /* Terrain generation stuff */
    Renderer::Mesh       m_terrainMesh;
    Terrain::Terrain     m_terrain;      // holds heightmap and chunksize
    Noise::TerrainData   m_terrainData;  // < This holds default and nice configuration for the terrain
    unsigned int         m_terrainWidthInChunks = 10, m_terrainHeightInChunks = 10;
    int                  m_chunkSize = 16;

    /*  Water Stuff */    
    World::Water      m_water;
    World::Sky        m_sky;
    float             m_realTime = 0;

    /* Rendering stuff */
    Renderer::Frustum m_frustum;

    Renderer::Texture m_rockTexture = Renderer::Texture("res/textures/rock.jpg");
    Renderer::Texture m_grassTexture = Renderer::Texture("res/textures/rock.jpg");

    //=====================================================================================================================//

public:
    TestWater()
    {
        m_rockTexture.bind(0);
        m_grassTexture.bind(1);

        m_player.setPostion({ 100.f, 22.F , 100.f });
        m_player.updateCamera();

        int samplers[8] = { 0,1,2,3,4,5,6,7 };
        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);

        m_terrain = Terrain::generateTerrain(
            m_terrainData,
            m_terrainWidthInChunks,
            m_terrainHeightInChunks,
            m_chunkSize);

        m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());
        m_water.addSource(0);
    }
    void step(float delta) override
    {
        m_realTime += delta;
        m_player.step(delta);
        m_water.updateMoveFactor(delta);
        m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());
    }

    void renderScene()
    {
        Renderer::Camera& camera = m_player.getCamera();
        Renderer::clear();

        m_rockTexture.bind(0);
        m_grassTexture.bind(1);

        for (const auto& [position, chunk] : m_terrain.getChunks()) {
            const AABB& chunkAABB = chunk.getMesh().getBoundingBox();

            bool isVisible = m_frustum.isOnFrustum(chunkAABB);

            if (isVisible) {
                Renderer::renderMesh(camera, glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh());
            }
        }

        m_sky.render(camera, m_realTime);
    }

    void onRender() override
    {
        m_water.onRender([this]() -> void { renderScene(); }, getCamera() );
    }

    void onImGuiRender() override
    {
    }

    CAMERA_IS_PLAYER(m_player);
};