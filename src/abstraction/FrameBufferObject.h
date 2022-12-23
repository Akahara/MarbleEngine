#pragma once

#include "BufferObject.h"
#include "Texture.h"
#include "UnifiedRenderer.h"

#include <stack>
#include <map>

namespace Renderer {

class FrameBufferObject {
private:

  unsigned int m_renderID;
  unsigned int m_depthBufferID;

  Renderer::Texture* m_target;
  Renderer::Texture* m_depth;
  
  struct ViewPort {
	  float width;
	  float height;
  } m_viewPort ;


public:
  FrameBufferObject();
  ~FrameBufferObject();
  FrameBufferObject(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(const FrameBufferObject &) = delete;
  FrameBufferObject &operator=(FrameBufferObject &&moved) noexcept;
  FrameBufferObject(FrameBufferObject &&moved) noexcept;

  void bind() const;
  void bindCached() const; // do something else, this is to prevent looping between stack pushing and poping
 
  static void unbind();
  void destroy();

  void setTargetTexture(Texture &texture, unsigned int slot = 0);
  void setDepthTexture(Texture &texture);

  void assertIsValid() const;

  static void setViewport(unsigned int width, unsigned int height);
  static void setViewportToTexture(const Texture &texture);
  static void setViewportToWindow();

  void setViewportToTargetTexture();
  float getViewportHeight() const ;
  float getViewportWidth() const ;
  Renderer::Texture* getTarget() const { return m_target; }

};



class FBOStack {

private:
	

	std::stack<const FrameBufferObject*> m_stack;
	Renderer::BlitPass m_blit;

protected:
	
	static FBOStack* m_instance;
	FBOStack() {}


public:

	// singleton stuff
	FBOStack(const FBOStack&) = delete;
	void operator=(const FBOStack&) = delete;

	static FBOStack* getInstance() {
		if (m_instance == nullptr)
			m_instance = new FBOStack();

		return m_instance;
	}


	//////////////////////////////:

	// to be called when binding an fbo
	void pushFBO(const FrameBufferObject* fbo) 
	{
		m_stack.push(fbo);
	}

	// to be called when unbinding an fbo
	void popFBO() {
		
		Renderer::Texture* target = m_stack.top()->getTarget();
		m_stack.pop();

		if (m_stack.empty()) {
			FrameBufferObject::setViewportToWindow();
			return;
		}

		float width, height;
		width = m_stack.top()->getViewportWidth();
		height = m_stack.top()->getViewportHeight();
		
		m_stack.top()->bindCached(); // cest juste un bind classique qui ne push pas dans le stack
		FrameBufferObject::setViewport(width, height);
		//m_blit.doBlit(*target);
	}

	int getCount() const { return m_stack.size(); }
	void resetStack() {

		while (!m_stack.empty()) {
			m_stack.pop();
		}

	}
};

}


