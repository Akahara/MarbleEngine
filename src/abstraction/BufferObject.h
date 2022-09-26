#pragma once

namespace Renderer {

	class BufferObject
	{
	protected:
		unsigned int m_RenderID;

	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Delete() = 0;

		BufferObject() : m_RenderID(0) {}
		BufferObject(const BufferObject&) = delete;
		BufferObject& operator=(const BufferObject&) = delete;

		// Unsafe
		unsigned int getId() { return m_RenderID; }
	};

}
