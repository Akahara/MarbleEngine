#pragma once

#include "../Scene.h"
#include "../../abstraction/Renderer.h"
#include "../../abstraction/Cubemap.h"
#include "../../abstraction/Window.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../World/Player.h"
#include "../../Utils/Debug.h"
#include "../../Utils/AABB.h"
#include "../../Utils/Mathf.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class SunCameraHelper {
private:
  static constexpr glm::vec3 WORLD_UP = Renderer::Camera::UP;
  static constexpr float SUN_CAM_ZNEAR = .1f;
  // -- parameters
  glm::vec3 m_sunDir; // directed toward the sun

  // -- imgui controls
  float m_controlSunDir[2]; // yaw&pitch // TODO extract ImGui controls into a friend class

  // -- computed
  // Sun space: IJ(K)
  // k is the distance along the sunDir axis, which does technically not exist in sun space
  glm::vec3 I, J;
  float m_sunI;
  float m_sunJ;
  float m_sunNearK;
  float m_sunFarK;
  float m_cameraHalfWidth, m_cameraHalfHeight;

  Renderer::Camera m_sunCamera;
  glm::mat4x3 m_shadowMapProj;
public:
  SunCameraHelper()
    : m_sunCamera(),
    m_sunDir(1, 0, 0),
    m_cameraHalfWidth(1.f),
    m_cameraHalfHeight(1.f),
    m_controlSunDir(),
    m_sunI(0), m_sunJ(0), m_sunNearK(0), m_sunFarK(0), I(), J(),
    m_shadowMapProj()
  {
  }

  void setSunDirection(const glm::vec3 &direction)
  {
    m_sunDir = glm::normalize(direction);
  }

  void repositionSunCamera(const AABB &visibleBoundingBox)
  {
    I = glm::normalize(glm::cross(m_sunDir, WORLD_UP));
    J = glm::normalize(glm::cross(m_sunDir, I));
    glm::mat3 P_W2S = glm::transpose(glm::mat3(I, J, m_sunDir)); // world to sun space projection

    // find the minimal camera quad that covers the bounding box given the sun orientation
    glm::vec3 minA = {+INFINITY, +INFINITY, +INFINITY}, maxB{-INFINITY, -INFINITY, -INFINITY};
    for (glm::vec3 p : visibleBoundingBox) {
      glm::vec3 pInSunSpace = P_W2S * p;
      minA = glm::min(pInSunSpace, minA);
      maxB = glm::max(pInSunSpace, maxB);
    }
    m_sunI = (maxB.x + minA.x) * .5f;
    m_sunJ = (maxB.y + minA.y) * .5f;
    m_cameraHalfWidth = (maxB.x - minA.x) * .5f;
    m_cameraHalfHeight = (maxB.y - minA.y) * .5f;
    m_sunFarK = minA.z;
    m_sunNearK = maxB.z;
  }

  void ensureCanCastShadows(const AABB &box)
  {
    if (!doesAABBOverlapExtendedViewFrustum(box))
      return;
    for (glm::vec3 p : box) {
      float dist = glm::dot(m_sunDir, p);
      m_sunNearK = glm::max(dist, m_sunNearK);
    }
  }

  void recalculateSunCameraMatrix()
  {
    float camZFar = m_sunNearK - m_sunFarK + SUN_CAM_ZNEAR;
    Renderer::OrthographicProjection projection;
    projection.left   = -m_cameraHalfWidth;
    projection.right  = +m_cameraHalfWidth;
    projection.bottom = -m_cameraHalfHeight;
    projection.top    = +m_cameraHalfHeight;
    projection.zNear  = SUN_CAM_ZNEAR;
    projection.zFar   = camZFar;
    m_sunCamera.setPosition(m_sunI * I + m_sunJ * J + (m_sunNearK + SUN_CAM_ZNEAR) * m_sunDir);
    m_sunCamera.lookAt(m_sunCamera.getPosition() + m_sunDir);
    m_sunCamera.setProjection(projection);
    m_sunCamera.recalculateViewMatrix();
    m_sunCamera.recalculateViewProjectionMatrix();
    float fw = 2.f * m_cameraHalfWidth;
    float fh = 2.f * m_cameraHalfHeight;
    m_shadowMapProj = glm::mat4x3{
      -I.x / fw,                     -J.x / fh,                     -m_sunDir.x,
      -I.y / fw,                     -J.y / fh,                     -m_sunDir.y,
      -I.z / fw,                     -J.z / fh,                     -m_sunDir.z,
       1 - (fw * .5f - m_sunI) / fw,  1 - (fh * .5f - m_sunJ) / fh,  m_sunNearK + SUN_CAM_ZNEAR,
    };
  }

  const Renderer::Camera &getCamera() const { return m_sunCamera; }
  const glm::mat4x3 &getWorldToShadowMapProjectionMatrix() const { return m_shadowMapProj; }
  float getZNear() const { return SUN_CAM_ZNEAR; }
  float getZFar()  const { return m_sunNearK - m_sunFarK + SUN_CAM_ZNEAR; }

  bool renderImGuiControls()
  {
    if (ImGui::DragFloat2("Sun yaw&pitch", m_controlSunDir, .025f)) {
      setSunDirection(Mathf::unitVectorFromRotation(m_controlSunDir[0], m_controlSunDir[1]));
      return true;
    }
    return false;
  }

private:
  // returns true iff the given bounding box overlaps with the 3d box generated from the
  // sun's camera far plane (quad) facing toward the sun and going to infinity
  bool doesAABBOverlapExtendedViewFrustum(const AABB &aabb)
  {
    return true; // TODO
  }
};

// TODO move these debug methods elsewhere
static void renderAABBDebugOutline(const Renderer::Camera &camera, const AABB &aabb, const glm::vec4 &color={1.f, 1.f, 0.f, 1.f})
{
  glm::vec3 o = aabb.getOrigin();
  glm::vec3 x = { aabb.getSize().x, 0, 0 };
  glm::vec3 y = { 0, aabb.getSize().y, 0 };
  glm::vec3 z = { 0, 0, aabb.getSize().z };
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o,         o + x,         color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o + y,     o + x + y,     color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o     + z, o + x     + z, color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o + y + z, o + x + y + z, color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o,         o + y,         color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o + x,     o + y + x,     color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o     + z, o + y     + z, color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o + x + z, o + y + x + z, color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o,         o + z,         color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o + x,     o + z + x,     color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o     + y, o + z     + y, color);
  Renderer::renderDebugLine(camera.getViewProjectionMatrix(), o + x + y, o + z + x + y, color);
}

static void renderOrthoCameraDebugOutline(const Renderer::Camera &viewCamera, const Renderer::Camera &outlinedCamera)
{
  const Renderer::OrthographicProjection &proj = outlinedCamera.getProjection<Renderer::OrthographicProjection>();
  const glm::vec3 pos = outlinedCamera.getPosition();
  glm::vec3 I = outlinedCamera.getRight();
  glm::vec3 J = outlinedCamera.getUp();
  glm::vec3 F = outlinedCamera.getForward();
  float zNear = proj.zNear;
  float zFar  = proj.zFar;
  glm::vec3 p1 = pos + I * proj.right + J * proj.top;
  glm::vec3 p2 = pos + I * proj.right - J * proj.top;
  glm::vec3 p3 = pos - I * proj.right - J * proj.top;
  glm::vec3 p4 = pos - I * proj.right + J * proj.top;

  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + F * zFar, { 1.f, 0.f, .0f, 1.f }); // dir
  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + Renderer::Camera::UP, { 1.f, 1.f, .3f, 1.f }); // up
  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + I, { .5f, 1.f, .5f, 1.f });
  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), pos, pos + J, { 1.f, .5f, .5f, 1.f });
  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p1, p1 + F * zFar, { .5f, .5f, .5f, 1.f });
  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p2, p2 + F * zFar, { .5f, .5f, .5f, 1.f });
  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p3, p3 + F * zFar, { .5f, .5f, .5f, 1.f });
  Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p4, p4 + F * zFar, { .5f, .5f, .5f, 1.f });
  for (float z = zNear; z < zFar; z += 5) {
    Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p1 + z * F, p2 + z * F, { .5f, .5f, .5f, 1.f });
    Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p2 + z * F, p3 + z * F, { .5f, .5f, .5f, 1.f });
    Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p3 + z * F, p4 + z * F, { .5f, .5f, .5f, 1.f });
    Renderer::renderDebugLine(viewCamera.getViewProjectionMatrix(), p4 + z * F, p1 + z * F, { .5f, .5f, .5f, 1.f });
  }
  Renderer::renderDebugCube(viewCamera.getViewProjectionMatrix(), pos, { .1f, .1f, .1f });

}

static void renderCameraDebugOutline(const Renderer::Camera &viewCamera, const Renderer::Camera &outlinedCamera)
{
  switch (outlinedCamera.getProjectionType()) {
  case Renderer::CameraProjection::ORTHOGRAPHIC: renderOrthoCameraDebugOutline(viewCamera, outlinedCamera); break;
  case Renderer::CameraProjection::PERSPECTIVE:  throw std::exception("Unimplemented");                     break;
  default:                                       throw std::exception("Unreachable");                       break;
  }
}

class TestShadowsScene : public Scene {
private:
  Renderer::Cubemap  m_skybox;
  Player             m_player;
  Renderer::Mesh     m_mesh1;
  Renderer::Shader   m_shader;

  SunCameraHelper    m_sunCameraHelper;
  AABB               m_visibleAABB = AABB::make_aabb({ -5, -.25f, -4 }, { 7, 3, 4 }).move({ 3, 0, 0 });

  Renderer::FrameBufferObject m_depthFBO;
  Renderer::Texture  m_depthTexture;

  Renderer::BlitPass m_depthTestBlitPass;
  bool               m_dbgDrawDepthBuffer = false;
  bool               m_animateSun = false;
public:
  TestShadowsScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" },
    m_mesh1(Renderer::loadMeshFromFile("res/models/floor.obj")),
    m_shader(Renderer::loadShaderFromFiles("res/shaders/shadows.vs", "res/shaders/shadows.fs")),
    m_depthTestBlitPass{ "res/shaders/shadows_testblitdepth.fs" }
  {
    m_sunCameraHelper.setSunDirection(glm::normalize(glm::vec3{ .5f, .1f, 0.f }));
    updateSunCamera();

    m_depthTexture = Renderer::Texture::createDepthTexture(1600 * 16 / 9, 1600);
    m_depthFBO.setDepthTexture(m_depthTexture);
  }

  void step(float delta) override
  {
    m_player.step(delta);

    if (m_animateSun) {
      static float time = 0;
      time += delta;
      m_sunCameraHelper.setSunDirection({ glm::cos(time), 1, glm::sin(time) });
      updateSunCamera();
    }
  }

  void renderScene(const Renderer::Camera &camera, bool depthPass)
  {
    m_depthTexture.bind();

    if (!depthPass) {
      Renderer::CubemapRenderer::drawCubemap(m_skybox, m_player.getCamera(), m_player.getPosition());
      renderCameraDebugOutline(camera, m_sunCameraHelper.getCamera());
      renderAABBDebugOutline(camera, m_visibleAABB);
    }

    m_shader.bind();
    m_shader.setUniformMat4f("u_M", glm::translate(glm::mat4(1.f), { 3, 0, 0 }));
    m_shader.setUniformMat4f("u_VP", camera.getViewProjectionMatrix());
    m_mesh1.draw();
  }

  void updateSunCamera()
  {
    m_sunCameraHelper.repositionSunCamera(m_visibleAABB);
    //m_sunCameraHelper.ensureCanCastShadows(AABB());
    m_sunCameraHelper.recalculateSunCameraMatrix();

    float zNear = m_sunCameraHelper.getZNear();
    float zFar = m_sunCameraHelper.getZFar();
    m_depthTestBlitPass.getShader().bind();
    m_depthTestBlitPass.getShader().setUniform1f("u_zNear", zNear);
    m_depthTestBlitPass.getShader().setUniform1f("u_zFar", zFar);
    m_shader.bind();
    m_shader.setUniformMat4x3f("u_shadowMapProj", m_sunCameraHelper.getWorldToShadowMapProjectionMatrix());
    m_shader.setUniform2f("u_shadowMapOrthoZRange", zNear, zFar);
  }

  void onRender() override
  {
    m_depthFBO.bind();
    m_depthFBO.setViewportToTexture(m_depthTexture);
    Renderer::clear();
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // do not draw color during a depth pass
    renderScene(m_sunCameraHelper.getCamera(), true);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    Renderer::FrameBufferObject::unbind();
    m_depthFBO.setViewportToWindow();

    Renderer::clear();
    if (m_dbgDrawDepthBuffer) {
      m_depthTestBlitPass.doBlit(m_depthTexture);
    } else {
      renderScene(m_player.getCamera(), false);
    }
  }

  void onImGuiRender() override
  {
    if (ImGui::Begin("Shadows")) {
      if (m_sunCameraHelper.renderImGuiControls())
        updateSunCamera();

      ImGui::Checkbox("draw depth", &m_dbgDrawDepthBuffer);
      ImGui::Checkbox("animate sun", &m_animateSun);
    }

    ImGui::End();
  }
};