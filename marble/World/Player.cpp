#include "Player.h"

#include "../abstraction/Inputs.h"
#include "../abstraction/Camera.h"
#include "../Utils/Mathf.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

static constexpr glm::vec3 UP{ 0, 1, 0 };

Player::Player()
  : m_camera()
{
  m_camera.setProjection(Renderer::PerspectiveProjection{ Mathf::PI / 3.f, 16.f / 9.f });

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

  m_camera.setYaw(m_camera.getYaw() - rotationMotion.x);
  m_camera.setPitch(std::max(-Mathf::PI * .499f, std::min(+Mathf::PI * .499f, m_camera.getPitch() + rotationMotion.y)));
  m_camera.setPosition(m_camera.getPosition() + motion * delta * speed);

  if (Inputs::isKeyPressed('Q')) {
    m_camera.setYaw(0);
    m_camera.setPitch(0);
    m_camera.setPosition({ 10, 10, 10 });
    updateCamera();
  }

  if(motion != glm::vec3(0) || rotationMotion != glm::vec2(0))
    updateCamera();
}

void Player::updateCamera()
{
  m_camera.recalculateViewMatrix();
  m_camera.recalculateViewProjectionMatrix();
}
