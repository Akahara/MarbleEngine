#pragma once

#define GLFW_INCLUDE_NONE
#include<glad/glad.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer {


	struct VertexBufferElement {

		unsigned int type;
		unsigned int count;
		unsigned char normalized;

		static unsigned int GetSizeOfType(unsigned int type) {

			switch (type) 
			{
				case GL_FLOAT:				return 4;
				case GL_UNSIGNED_INT:		return 4;
				case GL_UNSIGNED_BYTE:		return 1;
			}

			return 0;

		}

	};


	class VertexBufferLayout
	{
	private:
		std::vector<VertexBufferElement> m_Elements;
		unsigned int m_Stride;

	public:

		VertexBufferLayout() : m_Stride(0) {}

		template<typename T>
		void push(unsigned int count) {
			
		//	static_assert(false);

		}

		template<>
		void push<float>(unsigned int count) {

			m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
			m_Stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count; // 4 bytes

		}

		template<>
		void push<glm::vec2>(unsigned int count) {

			m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
			m_Stride += 2 * sizeof(GL_FLOAT) * count; // 4 bytes

		}

		template<>
		void push<glm::vec3>(unsigned int count) {

			m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
			m_Stride += 3 * sizeof(GL_FLOAT) * count; // 4 bytes

		}

		template<>
		void push<glm::vec4>(unsigned int count) {

			m_Elements.push_back({ GL_FLOAT, count, GL_FALSE });
			m_Stride += 4 * sizeof(GL_FLOAT) * count; // 4 bytes

		}

		template<>
		void push<unsigned int>(unsigned int count) {
			m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
			m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
		}

		template<>
		void push<unsigned char*>(unsigned int count) {
			m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
			m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
		}


		inline const std::vector<VertexBufferElement>& getElements() const { return m_Elements; }
		inline unsigned int getStride() const { return m_Stride; }

	};

}
