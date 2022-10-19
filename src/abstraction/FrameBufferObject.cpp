#include "FrameBufferObject.h"

#include <glad/glad.h>
#include <assert.h>

#include "Window.h"

namespace Renderer {

FrameBufferObject::FrameBufferObject()
  : m_depthBufferID(0)
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
{
  m_renderID = moved.m_renderID;
  m_depthBufferID = moved.m_depthBufferID;
  moved.m_renderID = 0;
  moved.m_depthBufferID = 0;
}

void FrameBufferObject::bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_renderID);
}

void FrameBufferObject::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObject::destroy()
{
  glDeleteFramebuffers(1, &m_renderID);
  glDeleteTextures(1, &m_depthBufferID);
  m_renderID = 0;
  m_depthBufferID = 0;
}

void FrameBufferObject::setTargetTexture(Texture &texture)
{
  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getId(), 0);
  assertIsValid();
  unbind();
}

void FrameBufferObject::setDepthTexture(Texture &texture)
{
  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.getId(), 0);
  assertIsValid();
  unbind();
}

void FrameBufferObject::assertIsValid() const
{
  bind();
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void FrameBufferObject::setViewport(unsigned int width, unsigned int height)
{
  glViewport(0, 0, width, height);
}

void FrameBufferObject::setViewportToTexture(const Texture &texture)
{
  glViewport(0, 0, texture.getWidth(), texture.getHeight());
}

void FrameBufferObject::setViewportToWindow()
{
  glViewport(0, 0, Window::getWinWidth(), Window::getWinHeight());
}

}