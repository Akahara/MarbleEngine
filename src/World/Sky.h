#pragma once

#include "../abstraction/Cubemap.h"
#include "../abstraction/Texture.h"
#include "../abstraction/Mesh.h"
#include "Player.h"

namespace World {

/**
* The sky class is basically a fancy wrapper arround a cubemap. It also
* renders clouds.
* 
* Default cubemaps are available and can be picked using Sky#SkyboxesType.
* 
* Use Renderer::SkyRenderer#drawSkyClouds on a Sky instance to draw both
* the sky and clouds.
*/
class Sky {
private:
  std::unique_ptr<Renderer::Cubemap> m_skybox; // this has no default constructor but we need late init

public:
  enum SkyboxesType {
	DEFAULT,
	SAND,
	SNOW
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