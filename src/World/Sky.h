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

  void Step(float delta);
  void Render(const Player &player) const;
};

}

namespace Renderer::SkyRenderer {

void Init();
void DrawSkyClouds(const Texture &cloudsTexture, const Player &player);
//void Shutdown();

}