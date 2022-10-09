#include "Camera.h"

namespace Renderer {

Camera::Camera(float left, float right, float bottom, float top)
  : m_View(1.0f)
{
  SetOrthoProjection(left, right, bottom, top);
}

Camera::Camera(float fovy, float aspect)
  : m_View(1.0f)
{
  SetPerspectiveProjection(fovy, aspect);
}

Camera::Camera()
  : m_View(1.0f),
  m_Projection(1.0f),
  m_ViewProjectionMatrix(1.0f)
{
}

void Camera::SetOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
  SetProjection(glm::ortho(left, right, bottom, top, zNear, zFar));
}

void Camera::SetPerspectiveProjection(float fovy, float aspect, float zNear, float zFar)
{
  SetProjection(glm::perspective(fovy, aspect, zNear, zFar));
}

void Camera::SetProjection(const glm::mat4 &value)
{
  m_Projection = value;
  RecalculateViewProjectionMatrix();
}

void Camera::SetView(const glm::mat4 &value)
{
  m_View = value;
  RecalculateViewProjectionMatrix();
}

}