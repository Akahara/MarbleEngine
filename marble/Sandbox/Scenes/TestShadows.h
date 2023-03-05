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
#include "../../World/SunCameraHelper.h"
#include "../../Utils/Debug.h"
#include "../../Utils/AABB.h"
#include "../../Utils/Mathf.h"

class TestShadowsScene : public Scene {
private:
  Player             m_player;
  Renderer::Mesh     m_visibleMesh;
  Renderer::InstancedMesh m_cubesInstancedMesh;
  std::shared_ptr<Renderer::Shader> m_shaders[2];
  World::Sky         m_sky;

  SunCameraHelper    m_sunCameraHelper;
  AABB               m_visibleAABB = AABB::make_aabb({ -2, -.25f, -4 }, { 9, 3, 4 });
  std::vector<AABB>  m_cubes;

  Renderer::FrameBufferObject m_depthFBO;
  std::shared_ptr<Renderer::Texture> m_depthTexture;
  static constexpr int        m_depthTextureSlot = 0;

  Renderer::BlitPass m_depthTestBlitPass;
  bool               m_dbgDrawDepthBuffer = false;
  bool               m_animateSun = false;
  float              m_realTime = 0;
  bool               m_drawDebugLines = true;
public:
  TestShadowsScene() : 
    m_depthTestBlitPass{ "res/shaders/shadows_testblitdepth.fs" },
    m_depthTexture(std::make_shared<Renderer::Texture>(std::move(Renderer::Texture::createDepthTexture(1600 * 16 / 9, 1600))))
  {
    m_shaders[0] = Renderer::loadShaderFromFiles("res/shaders/shadows.vs", "res/shaders/shadows.fs");
    m_shaders[1] = Renderer::loadShaderFromFiles("res/shaders/shadows_instanced.vs", "res/shaders/shadows.fs");
    auto material = std::make_shared<Renderer::Material>();
    auto materialInstanced = std::make_shared<Renderer::Material>();
    material->shader = m_shaders[0];
    material->textures[m_depthTextureSlot] = m_depthTexture;
    materialInstanced->shader = m_shaders[1];
    materialInstanced->textures[m_depthTextureSlot] = m_depthTexture;
    m_visibleMesh = Renderer::Mesh(Renderer::loadModelFromFile("res/meshes/floor.obj"), material);
    m_visibleMesh.getTransform().position = { 3, 0, 0 };
    m_cubesInstancedMesh = Renderer::InstancedMesh(Renderer::createCubeModel(), materialInstanced, 0);
    for (auto &shader : m_shaders) {
      shader->bind();
      shader->setUniform1i("u_shadowMap", m_depthTextureSlot);
    }
    m_depthFBO.setDepthTexture(*m_depthTexture);
    m_sunCameraHelper.setSunDirection(glm::normalize(glm::vec3{ .5f, .1f, 0.f }));
    m_depthTestBlitPass.attachInputTexture(m_depthTexture, 0);

    updateSunCamera();
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
    if (!depthPass && m_drawDebugLines) {
      renderDebugCameraOutline(camera, m_sunCameraHelper.getCamera());
      renderAABBDebugOutline(camera, m_visibleAABB);

      for (const AABB &box : m_cubes)
        renderAABBDebugOutline(camera, box, m_sunCameraHelper.isBoxVisibleBySun(box) ? glm::vec4{1,0,0,1} : glm::vec4{100.f,1,0,1});
    }

    Renderer::renderMesh(camera, m_visibleMesh);
    Renderer::renderMeshInstanced(camera, m_cubesInstancedMesh);
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
    for (auto &shader : m_shaders) {
      shader->bind();
      shader->setUniformMat4x3f("u_shadowMapProj", m_sunCameraHelper.getWorldToShadowMapProjectionMatrix());
      shader->setUniform2f("u_shadowMapOrthoZRange", zNear, zFar);
      shader->unbind();
    }
  }

  void onRender() override
  {
    m_depthFBO.bind();
    m_depthFBO.setViewportToTexture(*m_depthTexture);
    Renderer::clear();
    Renderer::beginDepthPass();
    renderScene(m_sunCameraHelper.getCamera(), true);
    Renderer::beginColorPass();
    Renderer::FrameBufferObject::unbind();
    Renderer::FrameBufferObject::setViewportToWindow();

    if (m_dbgDrawDepthBuffer) {
      m_depthTestBlitPass.doBlit();
    } else {
      Renderer::clear();
      renderScene(m_player.getCamera(), false);
      m_sky.render(m_player.getCamera(), m_realTime);
    }
  }

  void onImGuiRender() override
  {
    if (ImGui::Begin("Shadows")) {
      if (m_sunCameraHelper.renderImGuiControls())
        updateSunCamera();

      ImGui::Checkbox("draw debug lines", &m_drawDebugLines);
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

    Renderer::BaseInstance *cubeInstances = new Renderer::BaseInstance[m_cubes.size()];
    size_t visibleCubeCount = 0;
    for (const AABB &box : m_cubes)
      cubeInstances[visibleCubeCount++] = { box.getOrigin() + box.getSize() * .5f, box.getSize() };
    m_cubesInstancedMesh.replaceInstances(cubeInstances, m_cubes.size());
    delete[] cubeInstances;
  }

  CAMERA_IS_PLAYER(m_player);
};