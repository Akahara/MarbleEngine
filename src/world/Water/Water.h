#pragma once

#include "WaterRenderer.h"
#include "WaterSource.h"

#include "../../abstraction/Camera.h"
#include "../../abstraction/Renderer.h"


#include <functional>
#include <vector>




class Water {

	WaterRenderer m_renderer ;
    std::vector<WaterSource*> m_sources; // Multiple sources is not implemented yet
    
public:



    void addSource(float level = 0, const glm::vec2& pos = { 0,0 }, float size = 10.F) {

        WaterSource source{ level, pos };
        source.setSize(size);
        m_sources.push_back(&source);


    }

    void updateMoveFactor(float deltaTime) {

        m_renderer.updateMoveFactor(deltaTime);

    }




	void onRender(const std::function<void()>& renderFn, Renderer::Camera& camera) {

        WaterSource m_source = *m_sources.at(0);

        Renderer::Renderer::clear();


        // Reflection

        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, 1, 0, -m_source.getHeight()));
        Renderer::getStandardMeshShader().unbind();




        float distance = (camera.getPosition().y - m_source.getHeight()) * 2;

        // place camera
        camera.moveCamera({ 0, -distance, 0 });
        camera.inversePitch();
        // Change view


        m_renderer.bindReflectionBuffer();
        renderFn();
        /*
*/
        camera.moveCamera({ 0, distance, 0 });
        camera.inversePitch();
        m_renderer.unbind();

        // ---

        // Refraction

        m_renderer.bindRefractionBuffer();

        Renderer::getStandardMeshShader().bind();
        Renderer::getStandardMeshShader().setUniform4f("u_plane", glm::vec4(0, -1, 0, m_source.getHeight()));
        Renderer::getStandardMeshShader().unbind();

        renderFn();
        m_renderer.unbind();
        glDisable(GL_CLIP_DISTANCE0);
        //m_waterRenderer.writeTexture();
        m_renderer.onRenderWater(m_sources, camera);


	}



};