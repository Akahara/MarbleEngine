#pragma once

#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer {

enum class CameraProjection : unsigned char {
  ORTHOGRAPHIC,
  PERSPECTIVE,
};

struct PerspectiveProjection {
  float fovy;
  float aspect;
  float zNear = .1f;
  float zFar = 1000.f;

  glm::mat4 computeProjectionMatrix() const;
};

struct OrthographicProjection {
  float left;
  float right;
  float bottom;
  float top;
  float zNear = .1f;
  float zFar = 1000.f;

  glm::mat4 computeProjectionMatrix() const;
};

class Camera {
public:
  static constexpr glm::vec3 UP{ 0, 1, 0 };
private:
  glm::vec3 m_position;
  float m_yaw, m_pitch;
  CameraProjection m_projectionType;
  union {
    OrthographicProjection orthographic;
    PerspectiveProjection perspective;
  } m_projection;
  // computed & cached
  glm::mat4 m_projectionMatrix;
  glm::mat4 m_viewMatrix;
  glm::mat4 m_viewProjectionMatrix;
public:
  Camera();

  void setProjection(const OrthographicProjection &projection);
  void setProjection(const PerspectiveProjection &projection);
  void setPosition(const glm::vec3 &position) { m_position = position; }
  void setYaw(float yaw) { m_yaw = yaw; }
  void setPitch(float pitch) { m_pitch = pitch; }
  void lookAt(const glm::vec3 &target);

  // must be called after a position/rotation update ! Do not forget to also recalculate the ViewProjection matrix !
  void recalculateViewMatrix();
  // must be called after a view/projection update !
  void recalculateViewProjectionMatrix();

  const glm::mat4 &getViewProjectionMatrix() const { return m_viewProjectionMatrix; }
  const glm::mat4 &getProjectionMatrix() const { return m_projectionMatrix; }
  const glm::mat4 &getViewMatrix() const { return m_viewMatrix; }
  const glm::vec3 &getPosition() const { return m_position; }
  float getYaw() const { return m_yaw; }
  float getPitch() const { return m_pitch; }

  CameraProjection getProjectionType() const { return m_projectionType; }
  template<class P>
  const P &getProjection() const;
  template<> const OrthographicProjection &getProjection<OrthographicProjection>() const
  { assert(m_projectionType == CameraProjection::ORTHOGRAPHIC); return m_projection.orthographic; }
  template<> const PerspectiveProjection &getProjection<PerspectiveProjection>() const
  { assert(m_projectionType == CameraProjection::PERSPECTIVE); return m_projection.perspective; }

  glm::vec3 getRight() const;
  glm::vec3 getUp() const;
  glm::vec3 getForward() const;
};
}


