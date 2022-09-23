#pragma once

#include <iostream>

#define GLFW_INCLUDE_NONE
#include<glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_FORCE_CTOR_INIT 

namespace Renderer {

	class Camera
	{
	public:

		Camera(float left, float right, float bottom, float top)
			:
			m_Projection(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)),
			m_ViewProjectionMatrix(1.0f),
			m_View(1.0f),
			m_Position({0.0f, 0.0f, 0.0f})
		{
			m_ViewProjectionMatrix = m_Projection * m_View;
		}
		~Camera() {} 

		void setProjection(const glm::mat4& value) {

			m_Projection = value;
			RecalculateViewProjectionMatrix();

		}

		void setView(const glm::mat4& value) {

			m_View = value;
			RecalculateViewProjectionMatrix();

		}

		void setPosition(const glm::vec3& position) {
			m_Position = position; 
			RecalculateViewProjectionMatrix();
		}
		void setRotation(float rotation) {
			m_Rotation = rotation; RecalculateViewProjectionMatrix();
		}


		const glm::vec3 getPosition() const{ return m_Position; }


		void RecalculateViewProjectionMatrix() {

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
				* glm::rotate(glm::mat4(1.0f), m_Rotation, glm::vec3(0, 0, 1));

			m_View = glm::inverse(transform);
			m_ViewProjectionMatrix = m_Projection * m_View;

			

		}

		const glm::mat4& getViewProjectionMatrix () const { return m_ViewProjectionMatrix;  }
		const glm::mat4& getProjectionMatrix () const { return m_Projection;  }
		const glm::mat4& getViewMatrix () const { return m_View;  }






	private:


		glm::mat4 m_View, m_Projection, m_ViewProjectionMatrix;
		glm::vec3 m_Position;
		float m_Rotation = 0.0f;


	};
}


