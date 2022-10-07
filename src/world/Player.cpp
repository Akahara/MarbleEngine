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
  m_camera.SetPerspectiveProjection(Mathf::PI/3.f, 16.f/9.f);
  // default position
  m_yaw = .8f;
  m_pitch = .5f;
  m_position = { 3, 3, 3 };

  UpdateCamera();
}

void Player::Step(float delta)
{
  glm::vec3 motion{ 0 };
  if (Inputs::IsKeyPressed('A'))
    motion -= GetRight();
  if (Inputs::IsKeyPressed('D'))
    motion += GetRight();
  if (Inputs::IsKeyPressed('W'))
    motion += GetForward();
  if (Inputs::IsKeyPressed('S'))
    motion -= GetForward();
  if (Inputs::IsKeyPressed(' '))
    motion += UP;
  if (Inputs::IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
    motion -= UP;

  float speed = 25.f;
  if (Inputs::IsKeyPressed(GLFW_KEY_TAB))
    speed *= 15.f;

  glm::vec2 rotationMotion = Inputs::GetMouseDelta() / Inputs::GetInputRange() * Mathf::PI;

  m_yaw -= rotationMotion.x;
  m_pitch = std::max(-Mathf::PI * .499f, std::min(+Mathf::PI * .499f, m_pitch - rotationMotion.y));
  m_position += motion * delta * speed;

  if (Inputs::IsKeyPressed('Q')) {
    std::cout << "Q KEY" << std::endl;
    m_yaw = 0;
    m_pitch = 0;
    m_position = { 0, 0, 0 };
    UpdateCamera();
  }

  if(motion != glm::vec3(0) || rotationMotion != glm::vec2(0))
    UpdateCamera();
}

glm::vec3 Player::GetForward() const
{
  return { -sin(m_yaw), 0, -cos(m_yaw) };
}

glm::vec3 Player::GetRight() const
{
  return glm::cross(GetForward(), UP);
}

void Player::UpdateCamera()
{
  //std::cout << "update camera" << std::endl;
  glm::mat4 view(1.f);
  view = glm::rotate(view, m_pitch, { 1, 0, 0 });
  view = glm::rotate(view, -m_yaw, UP);
  view = glm::translate(view, -m_position);
  m_camera.SetView(view);
}
