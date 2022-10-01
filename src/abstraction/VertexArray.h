#pragma once

#include "VertexBufferObject.h"
#include "IndexBufferObject.h"

namespace Renderer {

	class VertexArray
	{
	private:
		unsigned int m_RendererID;

	public:
		VertexArray();
		~VertexArray();

		VertexArray(const VertexArray&) = delete;
		VertexArray &operator=(const VertexArray&) = delete;
		VertexArray &operator=(VertexArray &&moved) noexcept;
		VertexArray(VertexArray &&moved) noexcept;

		void Bind() const;
		void Unbind() const;
		void Delete();

		void addBuffer(const VertexBufferObject& vb, const VertexBufferLayout& layout, const IndexBufferObject &ib);
		void SendToGPU(GLsizeiptr size, const void* data);
	};

}
