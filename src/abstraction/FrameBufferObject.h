#pragma once

#include "BufferObject.h"
#include "Texture.h"

namespace Renderer {

class FrameBufferObject : public BufferObject {
private:
  unsigned int m_depthBufferID;
public:
  FrameBufferObject();
  ~FrameBufferObject();
  FrameBufferObject(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(FrameBufferObject &&moved) noexcept;
  FrameBufferObject(FrameBufferObject &&moved) noexcept;

  void Bind() const override;
  void Unbind() const override;
  void Delete() override;

  void SetTargetTexture(Texture &texture);
  void SetDepthTexture(Texture &texture);

  void AssertIsValid() const;

  static void SetViewport(unsigned int width, unsigned int height);
  static void SetViewportToTexture(const Texture &texture);
  static void SetViewportToWindow();
};

}
