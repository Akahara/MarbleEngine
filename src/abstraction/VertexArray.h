#pragma once

#include "VertexBufferObject.h"
#include "VertexBufferLayout.h"

namespace Renderer {




	class VertexArray
	{
	private:

		unsigned int m_RendererID;


	public:
		VertexArray();
		~VertexArray();

		VertexArray(const VertexArray&) = delete;
		VertexArray& operator=(const VertexArray&) = delete;

		void Bind() const;
		void Unbind() const;

		void addBuffer(const VertexBufferObject& vb, const VertexBufferLayout& layout);
		void SendToGPU(GLsizeiptr size, const void* data);
	};

}
