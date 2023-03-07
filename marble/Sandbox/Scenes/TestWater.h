#pragma once

#include "../Scene.h"

#include <vector>

#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Mesh.h"
#include "../../World/Player.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/Noise.h"
#include "../../World/TerrainGeneration/Terrain.h"
#include "../../World/Water/WaterRenderer.h"
#include "../../World/Water/Water.h"
#include "../../Utils/AABB.h"
#include "../Scenes/TestShadows.h"

class TestWater : public Scene {
private:

    //=====================================================================================================================//

      /* Basic scene stuff */
    Player            m_player;
    bool              m_playerIsFlying = true;

    /* Terrain generation stuff */
    Renderer::Mesh       m_terrainMesh;
    Terrain::Terrain     m_terrain;      // holds heightmap and chunksize

    /*  Water Stuff */    
    World::Water      m_water;
    World::Sky        m_sky;
    float             m_realTime = 0;

    /* Rendering stuff */
    Renderer::Frustum m_frustum;

    Renderer::Texture m_rockTexture  = Renderer::Texture("res/textures/rock.jpg");
    Renderer::Texture m_grassTexture = Renderer::Texture("res/textures/rock.jpg");

    //=====================================================================================================================//

public:
    TestWater()
    {
        m_rockTexture.bind(0);
        m_grassTexture.bind(1);

        m_player.setPostion({ 100.f, 22.F , 100.f });
        m_player.updateCamera();

        m_terrain = Terrain::generateTerrain(
            Noise::TerrainData{}, // use default values
            10, 10, // generate 10x10=100 chunks
            16      // each chunk is 16x16
        );

        m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());
        m_water.addSource({40,12,40},{80,80});
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

        //m_rockTexture.bind(0);
        //m_grassTexture.bind(1);

        for (const auto& [position, chunk] : m_terrain.getChunks()) {
            if (m_frustum.isOnFrustum(chunk.getMesh().getModel()->getBoundingBox()))
                Renderer::renderMesh(camera, chunk.getMesh());
        }

        m_sky.render(camera, m_realTime);
    }

    void onRender() override
    {
        m_water.onRender([this]() { renderScene(); }, getCamera() );
    }

    void onImGuiRender() override
    {
    }

    CAMERA_IS_PLAYER(m_player);
};