#pragma once

#include "src/Sandbox/Scene.h"


#include "src/world/Player.h"
#include "src/world/Sky.h"

class Playground : public Scene {


private:

	World::Sky m_sky{World::Sky::SkyboxesType::SAND};
	Player m_player; // TODO put player in namespace for consistency

	float m_realTime = 0;


public:


	void step(float delta) override {

		m_player.step(delta);
		m_realTime += delta;

	}

	void onRender() override {

		Renderer::clear();
		m_sky.render(m_player.getCamera(), m_realTime, true);


	}


	void onImGuiRender() override {

		ImGui::Text("This is the playground for tests ! ");

	}

	CAMERA_IS_PLAYER(m_player);
};


