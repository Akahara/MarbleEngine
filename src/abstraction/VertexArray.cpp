#include "VertexArray.h"

namespace Renderer {

VertexArray::VertexArray() {
	glGenVertexArrays(1, &m_RendererID);
	glBindVertexArray(m_RendererID);
}

VertexArray::~VertexArray() {
  destroy();
}

VertexArray::VertexArray(VertexArray &&moved) noexcept {
  m_RendererID = moved.m_RendererID;
  moved.m_RendererID = 0;
}

VertexArray &VertexArray::operator=(VertexArray &&moved) noexcept
{
  destroy();
  new (this) VertexArray(std::move(moved));
  return *this;
}

void VertexArray::bind() const {
  glBindVertexArray(m_RendererID);
}

void VertexArray::unbind() {
  glBindVertexArray(0);
}

void VertexArray::destroy()
{
  glDeleteVertexArrays(1, &m_RendererID);
  m_RendererID = 0;
}

void VertexArray::addBuffer(const VertexBufferObject& vb, const VertexBufferLayout& layout, const IndexBufferObject &ib) {
	bind();
	vb.bind();
	ib.bind();

	const auto& elements = layout.getElements();
	unsigned int offset = 0;

	for (unsigned int i = 0; i < elements.size(); i++) {

		const auto& element = elements[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), *(const void**)&offset);

		offset += element.count * VertexBufferElement::getSizeOfType(element.type);
	}
}

void VertexArray::sendToGPU(GLsizeiptr size, const void* data) {
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

}
