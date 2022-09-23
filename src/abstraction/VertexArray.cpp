#include "VertexArray.h"
#include "Renderer.h"

namespace Renderer {



VertexArray::VertexArray() {

	GLCall(glGenVertexArrays(1, &m_RendererID));
	GLCall(glBindVertexArray(m_RendererID));

}

VertexArray::~VertexArray() {

	GLCall(glDeleteVertexArrays(1, &m_RendererID));


}

void VertexArray::Bind() const {
	GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const {
	GLCall(glBindVertexArray(0));

}

void VertexArray::addBuffer(const VertexBufferObject& vb, const VertexBufferLayout& layout) {


	Bind();
	vb.Bind();


	const auto& elements = layout.getElements();
	unsigned int offset = 0;

	for (unsigned int i = 0; i < elements.size(); i++) {

		const auto& element = elements[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride() , (const void*)offset));

		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);


		}
	}

void VertexArray::SendToGPU(GLsizeiptr size, const void* data) {

	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);


	}
}
