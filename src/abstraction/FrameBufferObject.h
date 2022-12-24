#pragma once

#include <stack>
#include <map>

#include "BufferObject.h"
#include "Texture.h"
#include "UnifiedRenderer.h"

namespace Renderer {

class FrameBufferObject {
private:
  unsigned int m_renderID;

  Renderer::Texture* m_target;
  Renderer::Texture* m_depth;
  
  struct ViewPort {
	  unsigned int width;
	  unsigned int height;
  } m_viewPort;

public:
  FrameBufferObject();
  ~FrameBufferObject();
  FrameBufferObject(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(FrameBufferObject &&moved) noexcept;
  FrameBufferObject(FrameBufferObject &&moved) noexcept;

  void bind() const;
  void bindCached() const; // do something else, this is to prevent looping between stack pushing and poping TODO make this a friend function
 
  static void unbind();
  void destroy();

  void setTargetTexture(Texture &texture, unsigned int slot = 0);
  void setDepthTexture(Texture &texture);

  void assertIsValid() const;

  static void setViewport(unsigned int width, unsigned int height);
  static void setViewportToTexture(const Texture &texture);
  static void setViewportToWindow();

  void setViewportToTargetTexture();
  unsigned int getViewportHeight() const { return m_viewPort.height; }
  unsigned int getViewportWidth() const { return m_viewPort.width; }
  Renderer::Texture* getTarget() const { return m_target; }
};


class FBOStack {
private:
	std::stack<const FrameBufferObject*> m_stack;
	Renderer::BlitPass m_blit;

	// singleton stuff
	FBOStack(const FBOStack &) = delete;
	void operator=(const FBOStack &) = delete;
	FBOStack() {}
public:
	static FBOStack& getInstance() {
	  static FBOStack singleton;
	  return singleton;
	}

	// to be called when binding an fbo
	void pushFBO(const FrameBufferObject *fbo);
	// to be called when unbinding an fbo
	void popFBO();
	size_t getCount() const { return m_stack.size(); }
	void resetStack();
};

}


