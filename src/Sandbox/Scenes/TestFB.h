#pragma once

#include "../Scene.h"

#include "../../abstraction/Renderer.h"
#include "../../abstraction/Camera.h"

#include "Test2D.h"
#include "Test3D.h"
#include "TestTerrain.h"

class BlitData {
private:
  Renderer::IndexBufferObject  keepAliveIBO;
  Renderer::VertexBufferObject keepAliveVBO;

  Renderer::Shader             shader;
  Renderer::VertexArray        vao;
public:
  BlitData()
  {
    shader = Renderer::LoadShaderFromFiles("res/shaders/blit.vs", "res/shaders/blit.fs");
    keepAliveIBO = Renderer::IndexBufferObject({ 0, 2, 1, 3, 2, 0 });
    vao.addBuffer(keepAliveVBO, Renderer::VertexBufferLayout{}, keepAliveIBO);
  }

  void DoBlit(const Renderer::Texture &renderTexture)
  {
    glDisable(GL_DEPTH_TEST);
    Renderer::Renderer::Clear();
    renderTexture.Bind();

    shader.Bind();
    vao.Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    renderTexture.Unbind();
    vao.Unbind();
    glEnable(GL_DEPTH_TEST);
  }
};

class TestFBScene : public Scene {
private:
  Scene                      *m_backingScene;
  Renderer::FrameBufferObject m_fb;
  Renderer::Texture           m_fbRenderTexture;
  Renderer::Texture           m_fbDepthTexture;
  BlitData                    m_blitData;
public:
  TestFBScene()
  {
    m_backingScene = new TestTerrainScene;
    m_fbRenderTexture = Renderer::Texture(16*5, 9*5);
    m_fbDepthTexture = Renderer::Texture::createDepthTexture(m_fbRenderTexture.GetWidth(), m_fbRenderTexture.GetHeight());
    m_fb.SetTargetTexture(m_fbRenderTexture);
    m_fb.SetDepthTexture(m_fbDepthTexture);
  }

  ~TestFBScene()
  {
    delete m_backingScene;
  }

  void Step(float delta) override
  {
    m_backingScene->Step(delta);
  }

  void OnRender() override
  {
    m_fb.Bind();
    Renderer::FrameBufferObject::SetViewport(m_fbRenderTexture);

    m_backingScene->OnRender();

    m_fb.Unbind();
    Renderer::FrameBufferObject::SetViewport(Window::getWinWidth(), Window::getWinHeight());

    m_blitData.DoBlit(m_fbRenderTexture);
  }

  void OnImGuiRender() override 
  {
    m_backingScene->OnImGuiRender();
  }
};