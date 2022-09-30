#pragma once

#include "../abstraction/Cubemap.h"
#include "../abstraction/Texture.h"

namespace World {

class Sky {
private:
  Renderer::Cubemap m_skybox;
  Renderer::Texture m_cloudsTexture;
public:
  Sky();

  void Step(float delta);
  void Render(const Renderer::Camera &camera, glm::vec3 playerPosition) const;
};

}