#include "FrameBufferObject.h"

#include <glad/glad.h>
#include <assert.h>

#include "Window.h"

namespace Renderer {

FrameBufferObject::FrameBufferObject()
  : m_depthBufferID(0)
{
  glGenFramebuffers(1, &m_RenderID);
}

FrameBufferObject::~FrameBufferObject()
{
  Delete();
}

FrameBufferObject &FrameBufferObject::operator=(FrameBufferObject &&moved) noexcept
{
  Delete();
  new (this)FrameBufferObject(std::move(moved));
  return *this;
}

FrameBufferObject::FrameBufferObject(FrameBufferObject &&moved) noexcept
{
  m_RenderID = moved.m_RenderID;
  m_depthBufferID = moved.m_depthBufferID;
  moved.m_RenderID = 0;
  moved.m_depthBufferID = 0;
}

void FrameBufferObject::Bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_RenderID);
}

void FrameBufferObject::Unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObject::Delete()
{
  glDeleteFramebuffers(1, &m_RenderID);
  glDeleteTextures(1, &m_depthBufferID);
  m_RenderID = 0;
  m_depthBufferID = 0;
}

void FrameBufferObject::SetTargetTexture(Texture &texture)
{
  Bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getId(), 0);
  AssertIsValid();
  Unbind();
}

void FrameBufferObject::SetDepthTexture(Texture &texture)
{
  Bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.getId(), 0);
  AssertIsValid();
  Unbind();
}

void FrameBufferObject::AssertIsValid() const
{
  Bind();
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void FrameBufferObject::SetViewport(unsigned int width, unsigned int height)
{
  glViewport(0, 0, width, height);
}

void FrameBufferObject::SetViewportToTexture(const Texture &texture)
{
  glViewport(0, 0, texture.GetWidth(), texture.GetHeight());
}

void FrameBufferObject::SetViewportToWindow()
{
  glViewport(0, 0, Window::getWinWidth(), Window::getWinHeight());
}

}