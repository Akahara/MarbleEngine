#pragma once

#include "../Scene.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Mesh.h"
#include "../../World/Player.h"
#include "../../World/TerrainGeneration/HeightMap.h"
#include "../../World/TerrainGeneration/MeshGenerator.h"
#include "../../World/TerrainGeneration/Noise.h"

#include "../../World/Water/WaterSource.h"
#include "../../World/Water/WaterRenderer.h"


#include "../../Utils/AABB.h"
#include "../Scenes/TestShadows.h"

#include <vector>

class TestWater : public Scene {


private:

    //=====================================================================================================================//

      /* Basic scene stuff */
    Player            m_player;
    bool              m_playerIsFlying = true;
    float             realTime = 0;
    Player            m_roguePlayer; // another player used to better visualize frustum culling
    bool              m_isRoguePlayerActive = false;

    /* Terrain generation stuff */
    Renderer::Mesh                    m_terrainMesh;
    Terrain::Terrain     m_terrain;      // holds heightmap and chunksize
    Terrain::TerrainData m_terrainData;  // < This holds default and nice configuration for the terrain
    unsigned int                      m_terrainWidthInChunks = 10, m_terrainHeightInChunks = 10;
    int                               m_chunkSize = 16;

    /*  Water Stuff     */    
    WaterRenderer m_waterRenderer;
    WaterSource m_waterSource{ 0, {0,0} };
    std::vector<WaterSource*> m_sources;
    glm::vec3 m_waterPos{ 0 };
    float m_height = 0;

    World::Sky        m_sky;

    float             m_realTime = 0;

    /* Rendering stuff */
    Renderer::Frustum     m_frustum;

    Renderer::Texture     m_rockTexture = Renderer::Texture("res/textures/rock.jpg");
    Renderer::Texture     m_grassTexture = Renderer::Texture("res/textures/grass6.jpg");

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
        m_sources.push_back(&m_waterSource);
        m_waterSource.setSize(160);
        m_waterSource.setPosition(glm::vec2{80});
       
        m_waterSource.setHeight(9.2f);
    }

    void step(float delta) override
    {

        m_realTime += delta;
        m_player.step(delta);
        m_waterRenderer.updateMoveFactor(delta);
        m_frustum = Renderer::Frustum::createFrustumFromPerspectiveCamera(m_player.getCamera());
    }

    void renderScene() {

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
        Renderer::clear();


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
        renderScene();
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

        renderScene();
        m_waterRenderer.unbind();


        //m_waterRenderer.writeTexture();
        renderScene();
        m_waterRenderer.onRenderWater(m_sources, m_player.getCamera());


        m_sky.render(m_player.getCamera(), m_realTime);

    }

    void onImGuiRender() override
    {
        if (ImGui::SliderFloat2("Water plane pos", &m_waterPos.x, 0, 100)) {
            m_waterSource.setPosition(m_waterPos);
        }
        if (ImGui::SliderFloat("Water plane pos", &m_height, 0, 100)) {
            m_waterSource.setHeight(m_height);
        }
        if (ImGui::Button("write text to files")) {
            m_waterRenderer.writeTexture();
        }
    }
};