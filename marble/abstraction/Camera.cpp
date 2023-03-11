#include "Camera.h"

#include "../Utils/Mathf.h"

namespace Renderer {

glm::mat4 PerspectiveProjection::computeProjectionMatrix() const
{
  return glm::perspective(fovy, aspect, zNear, zFar);
}

glm::mat4 OrthographicProjection::computeProjectionMatrix() const
{
  return glm::ortho(left, right, bottom, top, zNear, zFar);
}

Camera::Camera()
  :
  m_position(0, 0, 0),
  m_yaw(0),
  m_pitch(0),
  m_viewMatrix(1.0f),
  m_viewProjectionMatrix(1.0f),
  m_projectionMatrix(1.0f),
  m_projectionType(CameraProjection::ORTHOGRAPHIC),
  m_projection({ .orthographic{} })
{
}

void Camera::setProjection(const OrthographicProjection &projection)
{
  m_projection.orthographic = projection;
  m_projectionType = CameraProjection::ORTHOGRAPHIC;
  m_projectionMatrix = projection.computeProjectionMatrix();
}

void Camera::setProjection(const PerspectiveProjection &projection)
{
  m_projection.perspective = projection;
  m_projectionType = CameraProjection::PERSPECTIVE;
  m_projectionMatrix = projection.computeProjectionMatrix();
}

void Camera::recalculateViewMatrix()
{
  m_viewMatrix = glm::rotate(glm::mat4{1.f}, -m_pitch, {1, 0, 0});
  m_viewMatrix = glm::rotate(m_viewMatrix, -m_yaw, UP);
  m_viewMatrix = glm::translate(m_viewMatrix, -m_position);
}

void Camera::recalculateViewProjectionMatrix()
{
  m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

glm::vec3 Camera::getForward() const
{
  return Mathf::unitVectorFromRotation(m_yaw, m_pitch);
}

glm::vec3 Camera::getRight() const
{
  return glm::normalize(glm::cross(getForward(), UP));
}

glm::vec3 Camera::getUp() const
{
  return glm::normalize(glm::cross(getRight(), getForward())); // very ineficient implementaion
}

void Camera::lookAt(const glm::vec3 &target)
{
  glm::vec3 d = glm::normalize(target - m_position);
  m_pitch = glm::asin(d.y);
  m_yaw = glm::atan(-d.x, -d.z);
}

Frustum Frustum::createFrustumFromCamera(const Camera &cam)
{
  switch (cam.getProjectionType()) {
  case CameraProjection::ORTHOGRAPHIC: return createFrustumFromOrthographicCamera(cam);
  case CameraProjection::PERSPECTIVE:  return createFrustumFromPerspectiveCamera(cam);
  default: assert(false); return {}; // unknown camera projection
  }
}

Frustum Frustum::createFrustumFromOrthographicCamera(const Camera &cam)
{
  OrthographicProjection proj = cam.getProjection<OrthographicProjection>();
  Frustum frustum;
  glm::vec3 forward = cam.getForward();
  glm::vec3 right = cam.getRight();
  glm::vec3 up    = cam.getUp();
  glm::vec3 pos   = cam.getPosition();

  frustum.nearFace   = { pos + proj.zNear * forward, cam.getForward() };
  frustum.farFace    = { pos + proj.zFar * forward, -cam.getForward() };
  frustum.rightFace  = { pos + right * proj.right, right };
  frustum.leftFace   = { pos - right * proj.left, -right };
  frustum.topFace    = { pos + up * proj.top, up };
  frustum.bottomFace = { pos - up * proj.bottom, -up };

  return frustum;
}

Frustum Frustum::createFrustumFromPerspectiveCamera(const Camera &cam)
{
  PerspectiveProjection proj = cam.getProjection<PerspectiveProjection>();
  Frustum frustum;
  const float halfVSide = proj.zFar * tanf(proj.fovy * .5f);
  const float halfHSide = halfVSide * proj.aspect;
  const glm::vec3 frontMultFar = proj.zFar * cam.getForward();

  frustum.nearFace = { cam.getPosition() + proj.zNear * cam.getForward(), cam.getForward() };
  frustum.farFace = { cam.getPosition() + frontMultFar, -cam.getForward() };
  frustum.rightFace = { cam.getPosition(), glm::cross(cam.getUp(), frontMultFar + cam.getRight() * halfHSide) };
  frustum.leftFace = { cam.getPosition(), glm::cross(frontMultFar - cam.getRight() * halfHSide, cam.getUp()) };
  frustum.topFace = { cam.getPosition(), glm::cross(cam.getRight(), frontMultFar - cam.getUp() * halfVSide) };
  frustum.bottomFace = { cam.getPosition(), glm::cross(frontMultFar + cam.getUp() * halfVSide, cam.getRight()) };

  return frustum;
}

bool Frustum::isOnFrustum(const AABB &boudingBox) const
{
  return (
    isOnOrForwardPlan(leftFace,   boudingBox) &&
    isOnOrForwardPlan(rightFace,  boudingBox) &&
    isOnOrForwardPlan(topFace,    boudingBox) &&
    isOnOrForwardPlan(bottomFace, boudingBox) &&
    isOnOrForwardPlan(nearFace,   boudingBox) &&
    isOnOrForwardPlan(farFace,    boudingBox)
  );
}

bool Frustum::isOnOrForwardPlan(const Plan &plan, const AABB &boundingBox)
{
  // Compute the projection interval radius of b onto L(t) = b.c + t * p.n

  glm::vec3 center = boundingBox.getOrigin() + (boundingBox.getSize()) / glm::vec3(2);
  glm::vec3 e = (boundingBox.getOrigin() + boundingBox.getSize()) - center;

  const float r = e.x * std::abs(plan.normal.x) + e.y * std::abs(plan.normal.y) + e.z * std::abs(plan.normal.z);

  return -r <= plan.getSDToPlan(center);
}

}