#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Camera.h"

namespace Renderer {

class Cubemap {
private:
  unsigned int m_id;
public:
  Cubemap(const std::string &front, const std::string &back,
          const std::string &left,  const std::string &right,
          const std::string &top,   const std::string &bottom);
  ~Cubemap();

  void bind() const;
  static void unbind();

  Cubemap(const Cubemap &) = delete;
  Cubemap &operator=(const Cubemap &) = delete;
};


// TODO move the renderer
namespace CubemapRenderer {

void init();
void drawCubemap(const Cubemap &cubemap, const Camera &camera, const glm::vec3 &offset = { 0,0,0 });
void shutdown();

}

}