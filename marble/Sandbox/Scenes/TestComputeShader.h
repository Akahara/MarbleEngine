#pragma once

#include "../Scene.h"

#include <iostream>

#include "../../abstraction/ComputeShader.h"

class TestComputeShader : public Scene {
private:
    Renderer::ComputeShader  m_computeShader{ "res/shaders/test_compute.comp", glm::uvec2(10, 1) };

public:
    TestComputeShader()
    {
        float values[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        m_computeShader.use();
        m_computeShader.setValues(values);
    }

    void step(float delta) override
    {
        m_computeShader.use();
        m_computeShader.dispatch();
        m_computeShader.wait();
        auto data = m_computeShader.getValues();

        for (auto d : data)
            std::cout << d << " ";
        std::cout << std::endl;
    }

    void onRender() override
    {
    }

    void onImGuiRender() override
    {
    }

    CAMERA_NOT_DEFINED();
};