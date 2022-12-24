#pragma once


//
//  Use this file for the final presentation demo, keep it simple
//  I'll try to produce a good looking environment
//

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


class DemoScene : public Scene {

private:

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

    /* Rendering stuff */
    Renderer::Frustum     m_frustum;

    Renderer::Texture     m_rockTexture = Renderer::Texture("res/textures/rock.jpg");
    Renderer::Texture     m_grassTexture = Renderer::Texture("res/textures/grass6.jpg");

    bool                  m_renderChunks = 0;

    Renderer::TestUniform m_depthTestUniform;
    Renderer::TestUniform m_grassSteepnessTestUniform;


    std::vector<Light> m_lights;

    /* Water */

    WaterRenderer m_waterRenderer;
    WaterSource m_waterSource{ 0, {0,0} };
    std::vector<WaterSource*> m_sources;



    struct Sun {

        glm::vec3 position{ 0.f };
        float strength = 1.25f;

    } m_sun;



public:
    DemoScene() : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
    {
        m_rockTexture.bind(0);
        m_grassTexture.bind(1);

        int samplers[8] = { 0,1,2,3,4,5,6,7 };
        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform1iv("u_Textures2D", 8, samplers);
        Renderer::getStandardMeshShader().setUniform1f("u_Strength", m_sun.strength);

        m_terrain = TerrainMeshGenerator::generateTerrain(
            m_terrainData,
            m_terrainWidthInChunks,
            m_terrainHeightInChunks,
            m_chunkSize);



        m_depthTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_fogDamping", 3, .0001f);
        m_depthTestUniform.setValue(.003f, .01f, .013f);
        m_grassSteepnessTestUniform = Renderer::TestUniform(&Renderer::getStandardMeshShader(), "u_grassSteepness", 2, .01f);
        m_grassSteepnessTestUniform.setValue(.79f, 1.f);

        m_frustum = Renderer::Frustum::createFrustumFromCamera(m_player.getCamera());


        m_waterSource.setHeight(9.2f);

    }

    void step(float delta) override
    {
        realTime += delta;
        (m_isRoguePlayerActive ? m_roguePlayer : m_player).step(delta);
        if (!m_playerIsFlying) {
            glm::vec3 pos = m_player.getPosition();
            pos.y = m_terrain.getHeightMap().getHeightLerp(pos.x, pos.z) + 1.f;
            m_player.setPostion(pos);
            m_player.updateCamera();
        }

        m_frustum = Renderer::Frustum::createFrustumFromCamera(m_player.getCamera());

    }

    void renderFn() 
    {


        Renderer::Renderer::clear();
        Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera());

        m_rockTexture.bind(0);
        m_grassTexture.bind(1);

        // Terrain rendering with frustum culling
        for (const auto& [position, chunk] : m_terrain.getChunks()) {
            const AABB& chunkAABB = chunk.getMesh().getBoundingBox();
            bool isVisible = Renderer::Frustum::isOnFrustum(m_frustum, chunkAABB);

            if (isVisible) {
                Renderer::renderMesh(glm::vec3{ 0 }, glm::vec3{ 1 }, chunk.getMesh(), m_player.getCamera());
            }
        }
 

    }

    void onRender() override {
        Renderer::Renderer::clear();


        // Reflection        

        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, 1, 0, -m_waterSource.getHeight()));
        Renderer::getStandardMeshShader().unbind();

        float distance = (m_player.getCamera().getPosition().y - m_waterSource.getHeight()) * 2;

        // place camera
        m_player.moveCamera({ 0, -distance, 0 });
        m_player.inversePitch();
        m_player.updateCamera();
        // Change view


        m_waterRenderer.bindReflectionBuffer();
        renderFn();

        m_player.moveCamera({ 0, distance, 0 });
        m_player.inversePitch();
        m_player.updateCamera();
        m_waterRenderer.unbind();

        // ---

        // Refraction

        m_waterRenderer.bindRefractionBuffer();

        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, -1, 0, m_waterSource.getHeight()));
        Renderer::getStandardMeshShader().unbind();

        renderFn();
        m_waterRenderer.unbind();



        glDisable(GL_CLIP_DISTANCE0);



        //m_waterRenderer.writeTexture();
        renderFn();
        m_waterRenderer.onRenderWater(m_sources, m_player.getCamera());


    }

    void onImGuiRender() override {}

    CAMERA_IS_PLAYER(m_player);
};