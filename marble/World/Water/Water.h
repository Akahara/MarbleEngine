#pragma once

#include <functional>
#include <vector>

#include "WaterRenderer.h"

#include "../../abstraction/Camera.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../vendor/imgui/imgui.h"

namespace World {

class Water {
private:
  WaterRenderer m_renderer;
  std::vector<WaterSource> m_sources; // Multiple sources is not implemented yet

public:
  Water() {}
  Water &operator=(const Water &) = delete;
  Water(const Water &) = delete;

  void addSource(glm::vec3 position, glm::vec2 size) { m_sources.emplace_back(position, size); }
  void removeSource(unsigned int index) { assert(index < m_sources.size()); m_sources.erase(m_sources.begin() + index); }
  WaterSource &getSourceAt(unsigned int index) { return m_sources[index]; }

  void updateMoveFactor(float deltaTime) { m_renderer.updateMoveFactor(deltaTime); }

  void onRender(const std::function<void()> &renderFn, Renderer::Camera &camera);

  void onImguiRender() {}
};

}