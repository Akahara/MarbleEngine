#include "VertexArray.h"
#include "Renderer.h"

namespace Renderer {

VertexArray::VertexArray() {
	glGenVertexArrays(1, &m_RendererID);
	glBindVertexArray(m_RendererID);
}

VertexArray::~VertexArray() {
  Delete();
}

VertexArray::VertexArray(VertexArray &&moved) noexcept {
  m_RendererID = moved.m_RendererID;
  moved.m_RendererID = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&moved) noexcept
{
  Delete();
  new (this) VertexArray(std::move(moved));
  return *this;
}

void VertexArray::Bind() const {
  glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const {
  glBindVertexArray(0);
}

void VertexArray::Delete()
{
  glDeleteVertexArrays(1, &m_RendererID);
  m_RendererID = 0;
}

void VertexArray::addBuffer(const VertexBufferObject& vb, const VertexBufferLayout& layout) {
	Bind();
	vb.Bind();

	const auto& elements = layout.getElements();
	unsigned int offset = 0;

	for (unsigned int i = 0; i < elements.size(); i++) {

		const auto& element = elements[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), *(const void**)&offset);

		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}

void VertexArray::SendToGPU(GLsizeiptr size, const void* data) {
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

}
