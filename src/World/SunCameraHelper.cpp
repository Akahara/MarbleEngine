#include "SunCameraHelper.h"

#include "../vendor/imgui/imgui.h"
#include "../Utils/Mathf.h"

static constexpr glm::vec3 WORLD_UP = Renderer::Camera::UP;

SunCameraHelper::SunCameraHelper()
  : m_sunCamera(),
  m_state(HelperState::READY),
  m_sunDir(1, 0, 0),
  m_cameraHalfWidth(1.f),
  m_cameraHalfHeight(1.f),
  m_controlSunDir(),
  m_minReceivingPoint(std::numeric_limits<float>::max()),
  m_maxReceivingPoint(std::numeric_limits<float>::min()),
  m_sunI(0), m_sunJ(0), m_sunNearK(0), m_sunFarK(0), I(), J(),
  m_worldToSunProjection(),
  m_shadowMapProj()
{
}

void SunCameraHelper::setSunDirection(const glm::vec3 &direction)
{
  ensureState<HelperState::READY>();
  m_sunDir = glm::normalize(direction);
}

void SunCameraHelper::prepareSunCameraMovement()
{
  ensureState<HelperState::READY>();

  I = glm::normalize(glm::cross(m_sunDir, WORLD_UP));
  J = glm::normalize(glm::cross(m_sunDir, I));
  m_worldToSunProjection = glm::transpose(glm::mat3(I, J, m_sunDir)); // world to sun space projection
  m_minReceivingPoint = glm::vec3(std::numeric_limits<float>::max());
  m_maxReceivingPoint = glm::vec3(std::numeric_limits<float>::min());

  m_state = HelperState::RECEIVING_BOXES;
}

void SunCameraHelper::ensureCanReceiveShadows(const AABB &box)
{
  ensureState<HelperState::RECEIVING_BOXES>();

  // find the minimal camera quad that covers the bounding box given the sun orientation
  for (glm::vec3 p : box) {
    glm::vec3 pInSunSpace = m_worldToSunProjection * p;
    m_minReceivingPoint = glm::min(pInSunSpace, m_minReceivingPoint);
    m_maxReceivingPoint = glm::max(pInSunSpace, m_maxReceivingPoint);
  }
}

void SunCameraHelper::prepareSunCameraCasting()
{
  ensureState<HelperState::RECEIVING_BOXES>();

  if (m_minReceivingPoint.x > m_maxReceivingPoint.x) {
    // No receiving boxes specified, add a dummy one
    ensureCanReceiveShadows(AABB({ 1000,1000,1000 }, { 1,1,1 }));
  }

  m_sunI = (m_maxReceivingPoint.x + m_minReceivingPoint.x) * .5f;
  m_sunJ = (m_maxReceivingPoint.y + m_minReceivingPoint.y) * .5f;
  m_cameraHalfWidth = (m_maxReceivingPoint.x - m_minReceivingPoint.x) * .5f;
  m_cameraHalfHeight = (m_maxReceivingPoint.y - m_minReceivingPoint.y) * .5f;
  m_sunFarK = m_minReceivingPoint.z;
  m_sunNearK = m_maxReceivingPoint.z;

  m_state = HelperState::CASTING_BOXES;
}

void SunCameraHelper::ensureCanCastShadows(const AABB &box)
{
  ensureState<HelperState::CASTING_BOXES>();
  if (!doesAABBOverlapExtendedViewFrustum(box))
    return;
  for (glm::vec3 p : box) {
    float dist = glm::dot(m_sunDir, p);
    m_sunNearK = glm::max(dist, m_sunNearK);
  }
}

void SunCameraHelper::finishSunCameraMovement()
{
  ensureState<HelperState::CASTING_BOXES>();
  float camZFar = m_sunNearK - m_sunFarK + SUN_CAM_ZNEAR;
  Renderer::OrthographicProjection projection;
  projection.left = -m_cameraHalfWidth;
  projection.right = +m_cameraHalfWidth;
  projection.bottom = -m_cameraHalfHeight;
  projection.top = +m_cameraHalfHeight;
  projection.zNear = SUN_CAM_ZNEAR;
  projection.zFar = camZFar;
  m_sunCamera.setPosition(m_sunI * I + m_sunJ * J + (m_sunNearK + SUN_CAM_ZNEAR) * m_sunDir);
  m_sunCamera.lookAt(m_sunCamera.getPosition() - m_sunDir);
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
  m_state = HelperState::READY;
}

bool SunCameraHelper::isBoxVisibleBySun(const AABB &box) const
{
  ensureState<HelperState::READY>();
  return doesAABBOverlapExtendedViewFrustum(box);
}

bool SunCameraHelper::renderImGuiControls()
  {
  if (ImGui::DragFloat2("Sun yaw&pitch", m_controlSunDir, .025f)) {
    setSunDirection(Mathf::unitVectorFromRotation(m_controlSunDir[0], m_controlSunDir[1]));
    return true;
  }
  return false;
}

bool SunCameraHelper::doesAABBOverlapExtendedViewFrustum(const AABB &aabb) const
{
  // To find an intersection between an aabb and a cuboid extending to infinity in one direction is not an easy task...
  // the cuboid is defined by the sun's far plane spanning to infinity in direction of the sun's near plane
  // (remember that the purpose of this method is to find bounding boxes intersecting with this cuboid
  // to "adjust" (push back) the sun's near plane)
  
  // we'll make an approximation: there is an intersection <=> *the bounding sphere of the aabb* intersects with the cuboid
  BoundingSphere bs = aabb.getBoundingSphere();
  // the sphere is invariant to rotation, so we only need to project its center
  glm::vec3 projectedCenter = m_worldToSunProjection * bs.getCenter();

  // another approximation: if the closest point of the sphere to the sun far *plane* (not rect!) is outside of the cuboid, there is no intersection
  if (projectedCenter.z + bs.getRadius() < m_sunFarK)
    return false;

  // once the sphere is projected on the far plane a simple circle to rectangle collision detection can be used
  float dx = glm::abs(projectedCenter.x - m_sunI);
  float dy = glm::abs(projectedCenter.y - m_sunJ);

  if (dx > m_cameraHalfWidth + bs.getRadius() || dy > m_cameraHalfHeight + bs.getRadius())
    return false;
  if (dx <= m_cameraHalfWidth || dy <= m_cameraHalfHeight)
    return true;
  float corderDistance = (dx - m_cameraHalfWidth) * (dx - m_cameraHalfWidth) + (dy - m_cameraHalfHeight) * (dy - m_cameraHalfHeight);

  return corderDistance <= bs.getRadius() * bs.getRadius();
}
