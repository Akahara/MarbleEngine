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
    Renderer::TerrainMesh m_terrain;      // holds heightmap and chunksize

    /*  Water Stuff */    
    World::Water      m_water;
    World::Sky        m_sky;
    float             m_realTime = 0;

public:
    TestWater()
    {
        m_player.setPostion({ 100.f, 22.F , 100.f });
        m_player.updateCamera();

        auto terrainMaterial = std::make_shared<Renderer::Material>();
        terrainMaterial->shader = Renderer::getStandardMeshShader();
        terrainMaterial->textures[0] = std::make_shared<Renderer::Texture>("res/textures/rock.jpg");
        terrainMaterial->textures[1] = std::make_shared<Renderer::Texture>("res/textures/rock.jpg");
        m_terrain.setMaterial(terrainMaterial);
        m_terrain.rebuildMesh([](float x, float y) { return x + y*.5f; }, {0,0, 80,80});

        m_water.addSource({40,12,40},{80,80});
    }

    void step(float delta) override
    {
        m_realTime += delta;
        m_player.step(delta);
        m_water.updateMoveFactor(delta);
    }

    void renderScene()
    {
        Renderer::Camera& camera = m_player.getCamera();
        Renderer::clear();

        Renderer::renderMeshTerrain(camera, m_terrain);

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