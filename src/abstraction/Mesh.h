#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "VertexArray.h"
#include "Texture.h"

#include "../Utils/AABB.h"

namespace Renderer {

	struct Vertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		float texId = 0; // TODO figure out how/if? we can use texture ids in vertices
		glm::vec3 color = {1.0f, 0.f, 0.f};
	};

class Mesh {
private:
  VertexBufferObject	m_VBO;
  IndexBufferObject		m_IBO;
  VertexArray			m_VAO;
  unsigned int			m_verticesCount;
  AABB					m_boudingBox;

public:
  Mesh();
  Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);
  ~Mesh();
  Mesh(Mesh &&moved) noexcept;
  Mesh &operator=(Mesh &&moved) noexcept;
  Mesh &operator=(const Mesh &) = delete;
  Mesh(const Mesh &) = delete;

  unsigned int getVertexCount() const { return m_verticesCount; }
  const AABB &getBoundingBox() const { return m_boudingBox; }
  AABB getBoundingBoxInstance(glm::vec3 instancePosition, glm::vec3 instanceSize) const;

  void draw() const;
};


class NormalsMesh {
private:
  VertexBufferObject	m_VBO;
  IndexBufferObject		m_IBO;
  VertexArray			m_VAO;
  unsigned int			m_verticesCount;

public:
  NormalsMesh();
  NormalsMesh(const std::vector<Vertex> &vertices);
  ~NormalsMesh();
  NormalsMesh(NormalsMesh &&moved) noexcept;
  NormalsMesh &operator=(NormalsMesh &&moved) noexcept;
  NormalsMesh &operator=(const NormalsMesh &) = delete;
  NormalsMesh(const NormalsMesh &) = delete;

  unsigned int getVertexCount() const { return m_verticesCount; }

  void draw() const;
};

}
