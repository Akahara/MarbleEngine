#pragma once

#include <glm/vec3.hpp>

#include "../abstraction/Camera.h"

class Player {
public:
  Player();

  void step(float delta);

  const Renderer::Camera &getCamera() const { return m_camera; }
  glm::vec3 getForward() const { return { -sin(m_yaw), 0, -cos(m_yaw) }; }
  glm::vec3 getRight() const { return m_camera.getRight(); }
  glm::vec3 getPosition() const { return m_position; }
  glm::vec2 getRotation() const { return { m_yaw, m_pitch }; }

  // After moving the player #updateCamera() should be called
  void setPostion(const glm::vec3 &position) { m_position = position; }
  void setRotation(float yaw, float pitch) { m_yaw = yaw; m_pitch = pitch; }

  void updateCamera();
private:
  glm::vec3 m_position;
  float m_yaw, m_pitch;
  Renderer::Camera m_camera;
};