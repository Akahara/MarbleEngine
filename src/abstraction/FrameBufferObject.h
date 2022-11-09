#pragma once

#include "BufferObject.h"
#include "Texture.h"

namespace Renderer {

class FrameBufferObject {
private:
  unsigned int m_renderID;
  unsigned int m_depthBufferID;
public:
  FrameBufferObject();
  ~FrameBufferObject();
  FrameBufferObject(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(FrameBufferObject &&moved) noexcept;
  FrameBufferObject(FrameBufferObject &&moved) noexcept;

  void bind() const;
  static void unbind();
  void destroy();

  void setTargetTexture(const Texture &texture);
  void setDepthTexture(Texture &texture);

  void assertIsValid() const;

  static void setViewport(unsigned int width, unsigned int height);
  static void setViewportToTexture(const Texture &texture);
  static void setViewportToWindow();
};

}
