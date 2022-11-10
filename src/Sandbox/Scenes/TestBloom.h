#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"

#include "../../abstraction/pipeline/Bloom.h"

class TestBloomScene : public Scene {
private:


    Renderer::Cubemap  m_skybox;
    Player             m_player;
    Renderer::Mesh m_cubeMesh = Renderer::createCubeMesh();


public:
    TestBloomScene() :
      m_skybox{
     "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
     "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
     "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
    {

    }

    ~TestBloomScene()
    {
    }

    void step(float delta) override
    {
        m_player.step(delta);
    }

    void onRender() override
    {
        Renderer::clear();
        Renderer::renderMesh({ 0,0,0 }, glm::vec3(3), m_cubeMesh, m_player.getCamera());

    }

    void onImGuiRender() override
    {
    }
};