#pragma once

#include <iostream>

#include "Texture.h"
#include "TextureAtlas.h"

#include "Shader.h"
#include "IndexBufferObject.h"
#include "VertexArray.h"
#include "FrameBufferObject.h"
#include "Camera.h"

namespace Renderer {

class Renderer {
public:

  static void clear();
  static void draw(const VertexArray &va, const IndexBufferObject &ibo, const Shader &shader);

  static void init();
  static void shutdown();
  static void beginBatch(const Camera &camera);
  static void endBatch();
  static void flush();

  static void drawQuad(const glm::vec2& position, const glm::vec2& size, const Texture& texture);
 
};

}
