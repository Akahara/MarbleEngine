#pragma once

#include <string>

#include "Camera.h"

namespace Renderer {

/* Immediate wrapper of the GL concept */
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

}