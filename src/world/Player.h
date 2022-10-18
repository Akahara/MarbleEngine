#pragma once

#include <glm/vec3.hpp>

#include "../abstraction/Camera.h"

class Player {
public:
  Player();

  void Step(float delta);

  const Renderer::Camera &GetCamera() const { return m_camera; }
  glm::vec3 GetForward() const { return { -sin(m_yaw), 0, -cos(m_yaw) }; }
  glm::vec3 GetRight() const { return m_camera.getRight(); }
  glm::vec3 GetPosition() const { return m_position; }
  glm::vec2 GetRotation() const { return { m_yaw, m_pitch }; }

  // After moving the player #UpdateCamera() should be called
  void SetPostion(const glm::vec3 &position) { m_position = position; }
  void SetRotation(float yaw, float pitch) { m_yaw = yaw; m_pitch = pitch; }

  void UpdateCamera();
private:
  glm::vec3 m_position;
  float m_yaw, m_pitch;
  Renderer::Camera m_camera;
};