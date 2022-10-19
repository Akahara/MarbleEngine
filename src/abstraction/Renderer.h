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

  static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &colors);
  static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture> &texture, const glm::vec4 &colors = { 1.0f, 1.0f, 1.0f, 1.0f });
  static void drawQuadFromAtlas(const glm::vec3 &position, const glm::vec2 &size, const TextureAtlas &texture, int x, int y, const glm::vec4 &colors = { 1.0f,1.0f,1.0f,1.0f });
};

}
