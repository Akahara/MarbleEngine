#pragma once

#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer {

class Camera {
private:
  glm::mat4 m_View, m_Projection, m_ViewProjectionMatrix;
public:
  Camera(float left, float right, float bottom, float top);
  Camera(float fovy, float aspect);
  Camera();

  void SetOrthoProjection(float left, float right, float bottom, float top, float zNear=.1f, float zFar=1000.f);
  void SetPerspectiveProjection(float fovy, float aspect, float zNear = .1f, float zFar = 1000.f);
  void SetProjection(const glm::mat4 &value);
  void SetView(const glm::mat4 &value);

  inline void RecalculateViewProjectionMatrix()
  {
	m_ViewProjectionMatrix = m_Projection * m_View;
  }

  const glm::mat4 &getViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
  const glm::mat4 &getProjectionMatrix() const { return m_Projection; }
  const glm::mat4 &getViewMatrix() const { return m_View; }

};
}


