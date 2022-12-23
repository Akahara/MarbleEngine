#pragma once

#pragma once

#include "WaterRenderer.h"
#include "WaterSource.h"

#include "../../abstraction/Camera.h"
#include "../../abstraction/UnifiedRenderer.h"


#include <functional>
#include <vector>



namespace World {

class Water {

    WaterRenderer m_renderer;
    std::vector<WaterSource*> m_sources; // Multiple sources is not implemented yet

public:

    // The default valus are what worked nicely for A SPECIFIC TEST
    void addSource(float level = 9.2f, const glm::vec2& pos = { 80,80 }, float size = 160.F) {

        WaterSource* source = new WaterSource{ level, pos };
        source->setSize(size);
        m_sources.push_back(source);


    }

    void removeSource(int index) {
        delete m_sources.at(index);
        m_sources.erase(m_sources.begin() + index);
    }

    WaterSource* getSourceAt(int index) {
        return m_sources[index];
    }

    void updateMoveFactor(float deltaTime) {

        m_renderer.updateMoveFactor(deltaTime);

    }




    void onRender(const std::function<void()>& renderFn, Renderer::Camera& camera) {


        //renderFn();
        const WaterSource& m_source = *m_sources.at(0);

        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, 1, 0, -m_source.getHeight())); // clipping plane stuff
        Renderer::getStandardMeshShader().unbind();




        float distance = (camera.getPosition().y - m_source.getHeight()) * 2;

        // place camera
        camera.moveCamera({ 0, -distance, 0 });
        camera.inversePitch();

        camera.recalculateViewMatrix();
        camera.recalculateViewProjectionMatrix();


        m_renderer.bindReflectionBuffer();
        renderFn();

        // Change view

        camera.moveCamera({ 0, distance, 0 });
        camera.inversePitch();
        camera.recalculateViewMatrix();
        camera.recalculateViewProjectionMatrix();
        m_renderer.unbind();

        // ---

        // Refraction

        m_renderer.bindRefractionBuffer();

        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, -1, 0, m_source.getHeight()));
        Renderer::getStandardMeshShader().unbind();

        renderFn();
        m_renderer.unbind();


        //m_waterRenderer.writeTexture();
        renderFn();
        m_renderer.onRenderWater(m_sources, camera);

        /*
        */

    }



};
}