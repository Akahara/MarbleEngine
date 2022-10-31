#pragma once

#include "../Scene.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../../abstraction/ComputeShader.h"

#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"
#include "../../World/Player.h"

#include <fstream>

class TestComputeShader : public Scene {
private:

    Renderer::ComputeShader m_computeShader;

public:
    TestComputeShader()
    {
      // TODO cleanup
      std::ifstream computeFile{ "res/shaders/shader.comp" };
      std::stringstream buffer;
      buffer << computeFile.rdbuf();
      std::string fragmentCode = buffer.str();


        m_computeShader = Renderer::ComputeShader(fragmentCode, glm::uvec2(10, 1));
        m_computeShader.use();
        float values[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
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
};