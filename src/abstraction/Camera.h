#pragma once

#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Utils/AABB.h"

namespace Renderer {


enum class CameraProjection : unsigned char {
  ORTHOGRAPHIC,
  PERSPECTIVE,
};

struct PerspectiveProjection {
  float fovy;          // in radians, in range ]0,PI[
  float aspect;        // camera width/camera height
  float zNear = .1f;   // in world space
  float zFar = 1000.f; // in world space

  glm::mat4 computeProjectionMatrix() const;
};

struct OrthographicProjection {
  float left;          // in world space
  float right;         // in world space
  float bottom;        // in world space
  float top;           // in world space
  float zNear = .1f;   // in world space
  float zFar = 1000.f; // in world space

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
  void inversePitch() { m_pitch = -m_pitch; }
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

struct Plan {

    glm::vec3 normal = { 0.f, 1.f, 0.f };
    float distanceToOrigin = 0.f;

    Plan() {}

    Plan(const glm::vec3& pl, const glm::vec3 norm) :
        normal(glm::normalize(norm)),
        distanceToOrigin(glm::dot(normal, pl))
    {}

    float getSDToPlan(const glm::vec3& point) const {

        return glm::dot(normal, point) - distanceToOrigin;

    }

};

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Frustum {

    Plan topFace;
    Plan bottomFace;

    Plan rightFace;
    Plan leftFace;

    Plan farFace;
    Plan nearFace;


    static Frustum createFrustumFromCamera(const Camera& cam)

    {
        float aspect = cam.getProjection<PerspectiveProjection>().aspect;
        float fovY = cam.getProjection<PerspectiveProjection>().fovy;
        float zNear = cam.getProjection<PerspectiveProjection>().zNear;
        float zFar = cam.getProjection<PerspectiveProjection>().zFar;
        return createFrustumFromCamera(cam, aspect, fovY, zNear, zFar);

    }

    static Frustum createFrustumFromCamera(
        const Camera& cam, float aspect, float fovY,
        float zNear, float zFar)

    {
        Frustum     frustum;

        const float halfVSide = zFar * tanf(fovY * .5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * cam.getForward();


        frustum.nearFace = { cam.getPosition() + zNear * cam.getForward(),  cam.getForward() };

        frustum.farFace = { cam.getPosition() + frontMultFar, -cam.getForward() };
        frustum.rightFace = { cam.getPosition(),
                                glm::cross(cam.getUp(),frontMultFar + cam.getRight() * halfHSide) };
        frustum.leftFace = { cam.getPosition(),
                                glm::cross(frontMultFar - cam.getRight() * halfHSide, cam.getUp()) };
        frustum.topFace = { cam.getPosition(),
                                glm::cross(cam.getRight(), frontMultFar - cam.getUp() * halfVSide) };
        frustum.bottomFace = { cam.getPosition(),
                                glm::cross(frontMultFar + cam.getUp() * halfVSide, cam.getRight()) };


        return frustum;
    }

    static bool isOnFrustum(const Frustum& frustum, const AABB& boudingBox) {


        return (
            isOnOrForwardPlan(frustum.leftFace, boudingBox) &&
            isOnOrForwardPlan(frustum.rightFace, boudingBox) &&
            isOnOrForwardPlan(frustum.topFace, boudingBox) &&
            isOnOrForwardPlan(frustum.bottomFace, boudingBox) &&
            isOnOrForwardPlan(frustum.nearFace, boudingBox) &&
            isOnOrForwardPlan(frustum.farFace, boudingBox)
            );




    }

    static bool isOnOrForwardPlan(const Plan& plan, const AABB& boundingBox)
    {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n

        glm::vec3 center = boundingBox.getOrigin() + (boundingBox.getSize()) / glm::vec3(2);
        glm::vec3 e = ( boundingBox.getOrigin() +boundingBox.getSize()) - center;

        const float r = e.x * std::abs(plan.normal.x) +
            e.y * std::abs(plan.normal.y) + e.z * std::abs(plan.normal.z);

        return -r <= plan.getSDToPlan(center);
    }

};

}


