#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/Player.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class TestShadowsScene : public Scene {
private:
  Renderer::Cubemap m_skybox;
  Player            m_player;
  Renderer::Mesh    m_mesh1;
  Renderer::Shader  m_shader;
  glm::vec3         m_sunPos{10, 10, 10};
  glm::vec3         m_sunDir{-1, -1, -1};
  glm::vec3         m_sunUp{0, 1, 0};
  glm::vec2         m_sunCamSize{ 4.f*16.f/9.f, 4.f };
  float             m_sunNear = .1f;
  float             m_sunFar = 100.f;
  Renderer::Camera  m_sunCamera;
  Renderer::FrameBufferObject m_depthFBO;
  Renderer::Texture m_depthTexture;
  Renderer::BlitPass m_depthTestBlitPass;
  bool              m_dbgDrawDepthBuffer = false;
public:
  TestShadowsScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
      m_mesh1(Renderer::LoadMeshFromFile("res/models/floor.obj")),
      m_shader(Renderer::LoadShaderFromFiles("res/shaders/shadows.vs", "res/shaders/shadows.fs")),
      m_depthTestBlitPass{ "res/shaders/shadows_testblitdepth.fs" }
  {
    UpdateSunCamera();
    m_depthTexture = Renderer::Texture::createDepthTexture(600 * 16 / 9, 600);
    m_depthFBO.SetDepthTexture(m_depthTexture);
  }

  void Step(float delta) override
  {
    m_player.Step(delta);
  }

  void RenderStrangeMesh(const Renderer::Camera &camera)
  {
    m_shader.Bind();
    m_shader.SetUniformMat4f("u_M", glm::mat4(1));
    m_shader.SetUniformMat4f("u_VP", camera.getViewProjectionMatrix());
    m_mesh1.Draw();
  }

  void RenderScene(const Renderer::Camera &camera)
  {
    m_depthTexture.Bind();
    RenderStrangeMesh(camera);
  }

  void UpdateSunCamera()
  {
    glm::mat4 view{ 1.f };
    view = glm::lookAt(m_sunPos, m_sunPos + m_sunDir, m_sunUp);
    m_sunCamera.SetView(view);
    m_sunCamera.SetOrthoProjection(-m_sunCamSize.x, m_sunCamSize.x, -m_sunCamSize.y, m_sunCamSize.y, m_sunNear, m_sunFar);
    m_depthTestBlitPass.GetShader().Bind();
    m_depthTestBlitPass.GetShader().SetUniform1f("u_zNear", m_sunNear);
    m_depthTestBlitPass.GetShader().SetUniform1f("u_zFar", m_sunFar);

    glm::vec3 R = glm::normalize(glm::cross(m_sunDir, m_sunUp)) / m_sunCamSize.x;
    glm::vec3 L = glm::normalize(glm::cross(m_sunDir, R)) / m_sunCamSize.y;
    glm::vec2 Ps{ glm::dot(L, m_sunPos), glm::dot(R, m_sunPos) };
    m_shader.Bind();
    m_shader.SetUniform3f("u_L", L);
    m_shader.SetUniform3f("u_R", R);
    m_shader.SetUniform2f("u_Ps", Ps);
  }

  void DbgDrawDepthTexture()
  {
    m_depthTestBlitPass.DoBlit(m_depthTexture);
  }

  void OnRender() override
  {
    m_depthFBO.Bind();
    m_depthFBO.SetViewport(m_depthTexture);
    Renderer::Clear();
    //Renderer::CubemapRenderer::DrawCubemap(m_skybox, m_player.GetCamera(), m_player.GetPosition());
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    RenderScene(m_sunCamera);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    m_depthFBO.Unbind();
    m_depthFBO.SetViewport(Window::getWinWidth(), Window::getWinHeight());

    Renderer::Clear();
    if (m_dbgDrawDepthBuffer) {
      DbgDrawDepthTexture();
    } else {
      RenderScene(m_player.GetCamera());
    }
  }

  void OnImGuiRender() override
  {
    if (ImGui::Begin("Shadows")) {
      if (ImGui::DragFloat3("sunPos", (float *)&m_sunPos, .05f) +
          ImGui::DragFloat3("sunDir", (float *)&m_sunDir, .05f) +
          ImGui::DragFloat3("sunUp", (float *)&m_sunUp, .05f) +
          ImGui::DragFloat2("sunCamSize", (float *)&m_sunCamSize, .01f) +
          ImGui::DragFloat("sunZNear", &m_sunNear, .01f) +
          ImGui::DragFloat("sunZFar", &m_sunFar, .01f)) {
        UpdateSunCamera();
      }

      ImGui::Checkbox("draw depth", &m_dbgDrawDepthBuffer);
    }

    ImGui::End();
  }
};