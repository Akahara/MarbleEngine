#pragma once


#include "../../abstraction/Window.h"
#include "../../abstraction/DeferredRenderer.h"
#include "../Scene.h"
#include "../../World/Player.h"
#include "../../World/Sky.h"
#include "../../World/Props/PropsManager.h"

class DeferredScene : public Scene  {
private:

	std::shared_ptr<Renderer::Mesh> m_cube =   std::make_shared<Renderer::Mesh>(Renderer::createCubeMesh());
	std::shared_ptr<Renderer::Mesh> m_plane =  std::make_shared<Renderer::Mesh>(Renderer::createPlaneMesh());
	std::shared_ptr<Renderer::Mesh> m_sphere = std::make_shared<Renderer::Mesh>(Renderer::createSphereMesh(20));
	std::shared_ptr<Renderer::Mesh> m_mesh1 = std::make_shared<Renderer::Mesh>(Renderer::loadMeshFromFile("res/meshes/floor.obj"));
	

	Player m_player;
	
	World::Sky m_sky;
	World::PropsManager m_props;
	DeferredRenderer m_deferredRenderer;

	float m_realtime = 0;

public:
	DeferredScene()
	{

		m_props.feed(m_cube, {5,0,0}, {1,4,1});
		m_props.feed(m_plane, {0,-3,0}, {30,1,30});
		m_props.feed(m_sphere);
		m_props.feed(m_mesh1);

	}

	void step(float delta) override
	{
		m_realtime += delta;
		m_player.step(delta);
	}

	void renderFn(Renderer::Camera& camera) 
	{
		Renderer::clear();
		m_props.render(camera);
		//m_sky.render(m_player.getCamera(), m_realtime, false);

	}

	void onRender() override
	{
		m_deferredRenderer.render(
			// renderFn
			[&]() -> void
			{
				renderFn(m_player.getCamera());
			},
			// scene camera
			m_player.getCamera()
		);
	}

	void onImGuiRender() override
	{

		m_deferredRenderer.renderImGuiDebugWindow();
		m_props.onImGuiRender();
	}



};

