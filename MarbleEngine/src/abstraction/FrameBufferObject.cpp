#include "FrameBufferObject.h"

#include <glad/glad.h>
#include <assert.h>

#include "Window.h"

namespace Renderer {

FrameBufferObject::FrameBufferObject()
  : m_depth(nullptr), m_target(nullptr), m_viewPort({ 0,0 })
{
  glGenFramebuffers(1, &m_renderID);
}

FrameBufferObject::~FrameBufferObject()
{
  destroy();
}

FrameBufferObject &FrameBufferObject::operator=(FrameBufferObject &&moved) noexcept
{
  destroy();
  new (this)FrameBufferObject(std::move(moved));
  return *this;
}

FrameBufferObject::FrameBufferObject(FrameBufferObject &&moved) noexcept
  : m_depth(nullptr), m_target(nullptr), m_viewPort({0,0})
{
  m_renderID = moved.m_renderID;
  moved.m_renderID = 0;
}

void FrameBufferObject::bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);
  FBOStack::getInstance().pushFBO(this);
}

void FrameBufferObject::bindCached() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);
}

void FrameBufferObject::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  FBOStack::getInstance().popFBO();
  
}

void FrameBufferObject::destroy()
{
  glDeleteFramebuffers(1, &m_renderID);
  m_renderID = 0;
}

void FrameBufferObject::setTargetTexture(Texture &texture, unsigned int slot/*=0*/)
{
  bind();

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getId(), 0);
  m_viewPort.width = texture.getWidth();
  m_viewPort.height = texture.getHeight();
  m_target = &texture;
  assertIsValid();

  unbind();
}

void FrameBufferObject::setDepthTexture(Texture &texture)
{
  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.getId(), 0);
  m_depth = &texture;
  assertIsValid();
  unbind();
}

void FrameBufferObject::assertIsValid() const
{
  bind();
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  unbind();
}

void FrameBufferObject::setViewport(unsigned int width, unsigned int height)
{
  glViewport(0, 0, width, height);
}

void FrameBufferObject::setViewportToTexture(const Texture &texture)
{
  glViewport(0, 0, texture.getWidth(), texture.getHeight());
}

void FrameBufferObject::setViewportToTargetTexture()
{
  glViewport(0, 0, m_viewPort.width, m_viewPort.height);
}

void FrameBufferObject::setViewportToWindow()
{
  glViewport(0, 0, Window::getWinWidth(), Window::getWinHeight());
}


void FBOStack::pushFBO(const FrameBufferObject *fbo)
{
  m_stack.push(fbo);
}

void FBOStack::popFBO()
{
  m_stack.pop();

  if (m_stack.empty()) {
	FrameBufferObject::setViewportToWindow();
	return;
  }

  unsigned int width, height;
  width  = m_stack.top()->getViewportWidth();
  height = m_stack.top()->getViewportHeight();

  m_stack.top()->bindCached(); // bind without pushing to the stack
  FrameBufferObject::setViewport(width, height);
}

void FBOStack::resetStack()
{
  while (!m_stack.empty())
	m_stack.pop();
}

}