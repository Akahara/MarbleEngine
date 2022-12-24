#pragma once

#include "../abstraction/Cubemap.h"
#include "../abstraction/Texture.h"
#include "../abstraction/Mesh.h"
#include "Player.h"

namespace World {

class Sky {
private:
  std::unique_ptr<Renderer::Cubemap> m_skybox; // this has no default constructor but we need late init

public:
  enum SkyboxesType {
	DEFAULT,
	SAND
  };

  Sky(const SkyboxesType& style = DEFAULT);
  
  void render(const Renderer::Camera &camera, float time=0.f, bool withClouds=true) const;
};

}

namespace Renderer::SkyRenderer {

void init();
void drawSkyClouds(const Camera &camera, float time);
//void Shutdown(); // TODO restore this shutdown and move the renderer elsewhere

}