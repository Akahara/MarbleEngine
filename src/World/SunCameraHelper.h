#pragma once

#include <numeric>

#include <glm/glm.hpp>

#include "../abstraction/Camera.h"
#include "../Utils/AABB.h"

/*
 * This class is a helper for depth passes that finds the best camera position and near&far planes
 * to make sure wanted objects appear in the depthmap and objects that can cast shadows are considered.
 * 
 * Note: Objects are approximated by their bounding boxes, it does not matter much if the sun's frustum
 * is a bit too large.
 * 
 * To use this class:
 *   Call #setSunDirection and #prepareSunCameraMovement.
 *   Call #ensureCanReceiveShadows for each object that must be visible in the depthmap and
 *     #prepareSunCameraCasting after.
 *   Call #ensureCanCastShadows for each object that must be able to cast shadows and
 *     #finishSunCameraMovement after.
 *   Then you can retrieve the zNear, zFar, camera and world to shadowmap projection matrix
 *     for the depth pass.
 *   This cycle can be repeated any number of times.
 * 
 * - #isBoxVisibleBySun can be used to discard objets not in the sun's frustum during the depth pass.
 * 
 * Typically in a scene with a sun that casts shadows, objects that can receive shadows are
 * those visible by the main camera and objects that can cast shadows are all objects in between
 * the sun and visible objects.
 */
class SunCameraHelper {
private:
  static constexpr float SUN_CAM_ZNEAR = .1f;

  enum class HelperState : unsigned char {
    READY,            // before #prepareSunCameraMovement or after #finishSunCameraMovement
    RECEIVING_BOXES,  // after #prepareSunCameraMovement, #ensureCanReceiveShadow can be called
    CASTING_BOXES,    // after #prepareSunCameraCasting, #ensureCanCastShadow can be called
  };

  // -- parameters
  glm::vec3 m_sunDir; // directed toward the sun

  // -- imgui controls
  float m_controlSunDir[2]; // yaw&pitch // TODO extract ImGui controls into a friend class

  // -- computed
  // Sun space: IJ(K)
  // k is the distance along the sunDir axis, which does technically not exist in sun space
  glm::vec3 I, J;
  glm::mat3 m_worldToSunProjection;
  glm::vec3 m_minReceivingPoint, m_maxReceivingPoint;
  float m_sunI;
  float m_sunJ;
  float m_sunNearK;
  float m_sunFarK;
  float m_cameraHalfWidth, m_cameraHalfHeight;

  HelperState      m_state;
  Renderer::Camera m_sunCamera;
  glm::mat4x3      m_shadowMapProj;
public:
  SunCameraHelper();

  // in READY state
  void setSunDirection(const glm::vec3 &direction);
  void prepareSunCameraMovement();
  // in RECEIVING_BOXES state
  void ensureCanReceiveShadows(const AABB &box);
  void prepareSunCameraCasting();
  // in CASTING_BOXES state
  void ensureCanCastShadows(const AABB &box);
  void finishSunCameraMovement();

  // in READY state
  const Renderer::Camera &getCamera()                      const { ensureState<HelperState::READY>(); return m_sunCamera; }
  const glm::mat4x3 &getWorldToShadowMapProjectionMatrix() const { ensureState<HelperState::READY>(); return m_shadowMapProj; }
  float getZNear()                                         const { ensureState<HelperState::READY>(); return SUN_CAM_ZNEAR; }
  float getZFar()                                          const { ensureState<HelperState::READY>(); return m_sunNearK - m_sunFarK + SUN_CAM_ZNEAR; }
  // returns true iff the box overlaps the sun's view frustum, that is, it must be drawn during the depth pass
  bool isBoxVisibleBySun(const AABB &box) const;

  bool renderImGuiControls();
private:
  // returns true iff the given bounding box overlaps with the 3d box generated from the
  // sun's camera far plane (quad) facing toward the sun and going to infinity
  bool doesAABBOverlapExtendedViewFrustum(const AABB &aabb) const;

  template<HelperState S>
  void ensureState() const { assert(m_state == S); }
};
