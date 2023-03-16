#pragma once

#include <queue>
#include <vector>

#include "../../abstraction/Mesh.h"
#include "../../abstraction/Camera.h"

namespace World {

class PropsManager {
private:
	std::vector<std::shared_ptr<Renderer::Mesh>> m_props;
public:
  void clear();
  void feed(const std::shared_ptr<Renderer::Mesh> &mesh);
  void render(const Renderer::Camera &camera);
  void onImGuiRender();
};

}