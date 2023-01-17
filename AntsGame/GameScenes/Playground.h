#pragma once

#include "src/Sandbox/Scene.h"


#include "src/world/Player.h"
#include "src/world/Sky.h"

class Playground : public Scene {


private:

	World::Sky m_sky{World::Sky::SkyboxesType::SAND};
	Player m_player; // TODO put player in namespace for consistency

	Renderer::Mesh m_cube = Renderer::createCubeMesh();

	float m_realTime = 0;


public:


	void step(float delta) override {

		m_player.step(delta);
		m_realTime += delta;

	}

	void onRender() override {

		Renderer::clear();
		Renderer::renderMesh(getCamera(), { 0,0,0 }, { 1,1,1 }, m_cube);
		Renderer::renderAABBDebugOutline(getCamera(), m_cube.getBoundingBoxInstance({ 0,0,0 }, { 1,1,1 }));
		//m_sky.render(m_player.getCamera(), m_realTime, true);


	}


	void onImGuiRender() override {

		ImGui::Text("This is the playground for tests ! ");

	}

	CAMERA_IS_PLAYER(m_player);
};


