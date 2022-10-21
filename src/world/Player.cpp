#include "Player.h"

#include "../abstraction/Inputs.h"
#include "../Utils/Mathf.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

static constexpr glm::vec3 UP{ 0, 1, 0 };

Player::Player()
  :
  m_camera(),
  m_yaw(0),
  m_pitch(0),
  m_position(0, 0, 0)
{
  m_camera.setProjection(Renderer::PerspectiveProjection{ Mathf::PI / 3.f, 16.f / 9.f });
  // default position
  m_yaw = .8f;
  m_pitch = .5f;
  m_position = { 3, 3, 3 };

  updateCamera();
}

void Player::step(float delta)
{
  glm::vec3 motion{ 0 };
  if (Inputs::isKeyPressed('A'))
    motion -= getRight();
  if (Inputs::isKeyPressed('D'))
    motion += getRight();
  if (Inputs::isKeyPressed('W'))
    motion += getForward();
  if (Inputs::isKeyPressed('S'))
    motion -= getForward();
  if (Inputs::isKeyPressed(' '))
    motion += UP;
  if (Inputs::isKeyPressed(GLFW_KEY_LEFT_SHIFT))
    motion -= UP;

  float speed = 25.f;
  if (Inputs::isKeyPressed(GLFW_KEY_TAB))
    speed *= 15.f;

  glm::vec2 rotationMotion = Inputs::getMouseDelta() / Inputs::getInputRange() * Mathf::PI;

  m_yaw -= rotationMotion.x;
  m_pitch = std::max(-Mathf::PI * .499f, std::min(+Mathf::PI * .499f, m_pitch + rotationMotion.y));
  m_position += motion * delta * speed;

  if (Inputs::isKeyPressed('Q')) {
    m_yaw = 0;
    m_pitch = 0;
    m_position = { 10, 10, 10 };
    updateCamera();
  }

  if(motion != glm::vec3(0) || rotationMotion != glm::vec2(0))
    updateCamera();
}

void Player::updateCamera()
{
  m_camera.setPosition(m_position);
  m_camera.setYaw(m_yaw);
  m_camera.setPitch(m_pitch);
  m_camera.recalculateViewMatrix();
  m_camera.recalculateViewProjectionMatrix();
}
