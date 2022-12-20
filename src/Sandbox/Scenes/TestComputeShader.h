#pragma once

#include "../Scene.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../../abstraction/ComputeShader.h"

#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Camera.h"
#include "../../World/Player.h"


#include "../../abstraction/compute.h"

#include <fstream>

class TestComputeShader : public Scene {
private:

    // le mien
    Renderer::ComputeShader  m_computeShader{ "res/shaders/shader.comp", glm::uvec2(10, 1) };


public:
    TestComputeShader()
    {
      // TODO cleanup

        float values[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        m_computeShader.use();
        m_computeShader.set_values(values);

    }

    void step(float delta) override
    {

        m_computeShader.use();
        m_computeShader.dispatch();
        m_computeShader.wait();
        auto data = m_computeShader.get_values();

        for (auto d : data) {
            std::cout << d << " ";
        }
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