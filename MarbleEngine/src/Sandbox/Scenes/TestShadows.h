#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../Scene.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/Window.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"
#include "../../World/Player.h"
#include "../../World/Sky.h"
#include "../../World/SunCameraHelper.h"
#include "../../Utils/Debug.h"
#include "../../Utils/AABB.h"
#include "../../Utils/Mathf.h"

class TestShadowsScene : public Scene {
private:
  Player             m_player;
  Renderer::Mesh     m_mesh1;
  Renderer::Mesh     m_cubeMesh;
  Renderer::Shader   m_shader;
  World::Sky         m_sky;

  SunCameraHelper    m_sunCameraHelper;
  AABB               m_visibleAABB = AABB::make_aabb({ -2, -.25f, -4 }, { 9, 3, 4 });
  std::vector<AABB>  m_cubes;

  Renderer::FrameBufferObject m_depthFBO;
  Renderer::Texture           m_depthTexture;
  static constexpr int        m_depthTextureSlot = 1; // cannot use slot0 because the mesh overrides it with the "missing texture"

  Renderer::BlitPass m_depthTestBlitPass;
  bool               m_dbgDrawDepthBuffer = false;
  bool               m_animateSun = false;
  float              m_realTime = 0;
public:
  TestShadowsScene() : 
    m_mesh1(Renderer::loadMeshFromFile("res/meshes/floor.obj")),
    m_cubeMesh(Renderer::createCubeMesh()),
    m_shader(Renderer::loadShaderFromFiles("res/shaders/shadows.vs", "res/shaders/shadows.fs")),
    m_depthTestBlitPass{ "res/shaders/shadows_testblitdepth.fs" }
  {
    m_shader.bind();
    m_shader.setUniform1i("u_shadowMap", m_depthTextureSlot);

    m_sunCameraHelper.setSunDirection(glm::normalize(glm::vec3{ .5f, .1f, 0.f }));
    updateSunCamera();

    m_depthTexture = Renderer::Texture::createDepthTexture(1600 * 16 / 9, 1600);
    m_depthFBO.setDepthTexture(m_depthTexture);

    generateCubes();
  }

  void step(float delta) override
  {
    m_player.step(delta);
    m_realTime += delta;

    if (m_animateSun) {
      static float time = 0;
      time += delta;
      m_sunCameraHelper.setSunDirection({ glm::cos(time), 1, glm::sin(time) });
      updateSunCamera();
    }
  }

  void renderScene(const Renderer::Camera &camera, bool depthPass)
  {
    if (!depthPass) {
      renderDebugCameraOutline(camera, m_sunCameraHelper.getCamera());
      renderAABBDebugOutline(camera, m_visibleAABB);

      for (const AABB &box : m_cubes)
        renderAABBDebugOutline(camera, box, m_sunCameraHelper.isBoxVisibleBySun(box) ? glm::vec4{1,0,0,1} : glm::vec4{100.f,1,0,1});
    }

    m_depthTexture.bind(m_depthTextureSlot);

    // do not use Renderer::renderMesh because the standard mesh shader would be used
    m_shader.bind();
    m_shader.setUniformMat4f("u_M", glm::translate(glm::mat4(1.f), { 3, 0, 0 }));
    m_shader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
    m_mesh1.draw();

    for (const AABB &box : m_cubes) {
      if (depthPass && !m_sunCameraHelper.isBoxVisibleBySun(box))
        continue; // this skip might actually be slower than to draw the box anyways, isBoxVisibleBySun is not cheap
      glm::vec3 p = box.getOrigin() + box.getSize() * .5f;
      glm::mat4 M = glm::mat4(1.f);
      M = glm::translate(M, p);
      M = glm::scale(M, box.getSize());
      m_shader.setUniformMat4f("u_M", M);
      m_cubeMesh.draw();
    }
  }

  void updateSunCamera()
  {
    m_sunCameraHelper.prepareSunCameraMovement();
      m_sunCameraHelper.ensureCanReceiveShadows(m_visibleAABB);
    m_sunCameraHelper.prepareSunCameraCasting();
    for (const AABB &box : m_cubes)
      m_sunCameraHelper.ensureCanCastShadows(box);
    m_sunCameraHelper.finishSunCameraMovement();

    float zNear = m_sunCameraHelper.getZNear();
    float zFar = m_sunCameraHelper.getZFar();
    m_depthTestBlitPass.getShader().bind();
    m_depthTestBlitPass.getShader().setUniform1f("u_zNear", zNear);
    m_depthTestBlitPass.getShader().setUniform1f("u_zFar", zFar);
    m_shader.bind();
    m_shader.setUniformMat4x3f("u_shadowMapProj", m_sunCameraHelper.getWorldToShadowMapProjectionMatrix());
    m_shader.setUniform2f("u_shadowMapOrthoZRange", zNear, zFar);
    m_shader.unbind();
  }

  void onRender() override
  {
    m_depthFBO.bind();
    m_depthFBO.setViewportToTexture(m_depthTexture);
    Renderer::clear();
    Renderer::beginDepthPass();
    renderScene(m_sunCameraHelper.getCamera(), true);
    Renderer::beginColorPass();
    Renderer::FrameBufferObject::unbind();
    Renderer::FrameBufferObject::setViewportToWindow();

    Renderer::clear();
    if (m_dbgDrawDepthBuffer) {
      m_depthTexture.bind(0);
      m_depthTestBlitPass.doBlit();
    } else {
      renderScene(m_player.getCamera(), false);
    }
    m_sky.render(getCamera(), m_realTime);
  }

  void onImGuiRender() override
  {
    if (ImGui::Begin("Shadows")) {
      if (m_sunCameraHelper.renderImGuiControls())
        updateSunCamera();

      ImGui::Checkbox("draw depth", &m_dbgDrawDepthBuffer);
      ImGui::Checkbox("animate sun", &m_animateSun);

      if (ImGui::Button("clear cubes"))
        m_cubes.clear();
      if (ImGui::Button("add cubes"))
        generateCubes();
    }

    ImGui::End();
  }

  void generateCubes()
  {
    for (size_t i = 0; i < 20; i++) {
      glm::vec3 p = glm::ballRand(20.f) + glm::vec3(-2, 0, 0);
      glm::vec3 s = { glm::linearRand(.5f, 1.5f), glm::linearRand(.5f, 1.5f), glm::linearRand(.5f, 1.5f) };
      m_cubes.push_back(AABB(p, s));
    }
  }

  CAMERA_IS_PLAYER(m_player);
};