#pragma once


#include "../Scene.h"

#include <iostream>


#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Camera.h"
#include "../../World/Player.h"
#include "../../World/Sky.h"

class TestAtlas : public Scene {
private:

     Player m_player;
     World::Sky m_sky;

     Renderer::Mesh m_mesh = Renderer::loadMeshFromFile("res/meshes/what.obj");
     //Renderer::Mesh m_cube = Renderer::createCubeMesh();

     float m_realtime = 0;

public:
    TestAtlas()
    {

    }

    void step(float delta) override
    {
        m_player.step(delta);
        m_realtime += delta;
    }

    void onRender() override
    {
        Renderer::clear();
        Renderer::renderMesh(m_player.getCamera(), { 10,0,0 }, { 10,10,10 }, m_mesh);
        m_sky.render(getCamera(), m_realtime);
    }

    void onImGuiRender() override
    {
    }

    CAMERA_IS_PLAYER(m_player);
};