#pragma once

#include <glm/glm.hpp>

#include "../../abstraction/Mesh.h"
#include "../../abstraction/UnifiedRenderer.h"

namespace Renderer {
	struct Frustum;
}

class WaterSource
{
private:
	float          m_height;
	float          m_size = 10.F;

	glm::vec2      m_position;

	Renderer::Mesh m_mesh;

public:
    // TODO these constructors are very inconsistent on which members they initialize
	WaterSource() 
		: m_height(0), m_position{0.f,0.f}
	{ }

	WaterSource(float level, const glm::vec2& pos) 
		: m_height(level), m_position(pos), m_mesh(Renderer::createPlaneModel(), std::make_shared<Renderer::Material>()) // shouldn't size be set ?
	{
	}

	// dont ask me anything about this class
	WaterSource(const WaterSource& other)
		: m_height(other.m_height), m_position(other.m_position), m_size(other.m_size) // mesh cannot be copied and is not?
	{ }

	WaterSource &operator=(const WaterSource &other);

	//------------------------------------//

	void draw() const {
		//m_mesh.draw(); // FIX water source rendering
	}

	bool isOnFrustum(Renderer::Frustum& frustum) {
		return frustum.isOnFrustum(m_mesh.getModel()->getBoundingBox());
	}

	//------------------------------------//

	float     getHeight()   const { return m_height; }
	float     getSize()     const { return m_size; }
	glm::vec2 getPosition() const { return m_position; }

	void      setHeight(float level)            { m_height = level; }
	void      setSize(float size)               { m_size = size; }
	void      setPosition(const glm::vec2& pos) { m_position = pos; }
};

