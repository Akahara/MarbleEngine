#pragma once

#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer {

	class Camera
	{
	public:

		Camera(float left, float right, float bottom, float top)
			: m_View(1.0f)
		{
		    SetOrthoProjection(left, right, bottom, top);
		}

		Camera(float fovy, float aspect)
		  : m_View(1.0f)
		{
		  SetPerspectiveProjection(fovy, aspect);
		}

		Camera()
		  :
		  m_View(1.0f),
		  m_Projection(1.0f),
		  m_ViewProjectionMatrix(1.0f)
		{
		}

		void SetOrthoProjection(float left, float right, float bottom, float top)
		{
		  SetProjection(glm::ortho(left, right, bottom, top, -1.0f, 1.0f));
		}

		void SetPerspectiveProjection(float fovy, float aspect)
		{
		  SetProjection(glm::perspective(fovy, aspect, .1f, 2000.f));
		}

		void SetProjection(const glm::mat4& value) {
			m_Projection = value;
			RecalculateViewProjectionMatrix();
		}

		void SetView(const glm::mat4& value) {
			m_View = value;
			RecalculateViewProjectionMatrix();
		}

		inline void RecalculateViewProjectionMatrix() {
			m_ViewProjectionMatrix = m_Projection * m_View;
		}

		const glm::mat4& getViewProjectionMatrix () const { return m_ViewProjectionMatrix;  }
		const glm::mat4& getProjectionMatrix () const { return m_Projection;  }
		const glm::mat4& getViewMatrix () const { return m_View;  }

	private:
		glm::mat4 m_View, m_Projection, m_ViewProjectionMatrix;
	};
}


