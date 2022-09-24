#pragma once

#include <glm/vec3.hpp>

#include "../abstraction/Camera.h"

class Player {
public:
  Player();

  void Step(float delta);

  const Renderer::Camera &GetCamera() const { return m_camera; }
  glm::vec3 GetForward() const;
  glm::vec3 GetRight() const;

private:
  void UpdateCamera();
private:
  glm::vec3 m_position;
  float m_yaw, m_pitch;
  Renderer::Camera m_camera;
};