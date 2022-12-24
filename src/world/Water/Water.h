#pragma once

#include <functional>
#include <vector>

#include "WaterRenderer.h"
#include "WaterSource.h"

#include "../../abstraction/Camera.h"
#include "../../abstraction/UnifiedRenderer.h"

namespace World {

class Water {
private:
  WaterRenderer m_renderer;
  std::vector<WaterSource*> m_sources; // Multiple sources is not implemented yet

public:
  Water() {}
  Water &operator=(const Water &) = delete;
  Water(const Water &) = delete;

  // The default valus are what worked nicely for A SPECIFIC TEST
  void addSource(float level = 9.2f, const glm::vec2 &pos = { 80,80 }, float size = 160.F);
  void removeSource(int index);

  WaterSource* getSourceAt(int index) { return m_sources[index]; }
  void updateMoveFactor(float deltaTime) { m_renderer.updateMoveFactor(deltaTime); }

  void onRender(const std::function<void()> &renderFn, Renderer::Camera &camera);
};

}