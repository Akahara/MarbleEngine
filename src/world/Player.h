#pragma once

#include <glm/vec3.hpp>

#include "../abstraction/Camera.h"

/*
* The player class is basically a fancy wrapper arround a camera, it also
* handles movement.
* 
* In scene classes simply having a player instance and calling #step on 
* every step cycle is enough to have access to #getCamera. To avoid extra
* matrix calculations, when moving the player or its rotation the camera
* projection and view matrices are not updated, do call #updateCamera after
* applying your changes.
*/
class Player {
public:
  Player();

  void step(float delta);

  const Renderer::Camera &getCamera() const { return m_camera; }
  Renderer::Camera &getCamera() { return m_camera; }
  glm::vec3 getForward() const { return { -sin(m_camera.getYaw()), 0, -cos(m_camera.getYaw()) }; }
  glm::vec3 getRight() const { return m_camera.getRight(); }
  glm::vec3 getPosition() const { return m_camera.getPosition(); }
  glm::vec2 getRotation() const { return { m_camera.getYaw(), m_camera.getPitch() }; }

  // After moving the player #updateCamera() should be called
  void setPostion(const glm::vec3 &position) { m_camera.setPosition(position); }
  void setRotation(float yaw, float pitch) { m_camera.setYaw(yaw); m_camera.setPitch(pitch); }
  void inversePitch() { m_camera.setPitch(-m_camera.getPitch()); }
  void moveCamera(const glm::vec3& delta) { m_camera.setPosition(m_camera.getPosition() + delta); m_camera.setPosition(m_camera.getPosition() + delta); }

  void updateCamera();
private:
  Renderer::Camera m_camera;
};