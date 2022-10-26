#pragma once

#include "../abstraction/Cubemap.h"
#include "../abstraction/Texture.h"
#include "../abstraction/Mesh.h"
#include "Player.h"

namespace World {

class Sky {
private:
  Renderer::Cubemap m_skybox;
public:
  Sky();

  void step(float delta);
  void render(const Renderer::Camera &camera, float time=0.f) const;
};

}

namespace Renderer::SkyRenderer {

void init();
void drawSkyClouds(const Camera &camera, float time);
//void Shutdown(); // TODO restore this shutdown and move the renderer elsewhere

}