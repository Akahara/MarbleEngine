#pragma once

#include "../abstraction/Cubemap.h"
#include "../abstraction/Texture.h"
#include "../abstraction/Mesh.h"
#include "Player.h"

namespace World {

class Sky {
private:
  Renderer::Cubemap m_skybox;
  Renderer::Texture m_cloudsTexture;
public:
  Sky();

  void step(float delta);
  void render(const Player &player) const;
};

}

namespace Renderer::SkyRenderer {

void init();
void drawSkyClouds(const Texture &cloudsTexture, const Player &player);
//void Shutdown(); // TODO restore this shutdown and move the renderer elsewhere

}