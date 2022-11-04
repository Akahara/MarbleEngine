#pragma once

#include "../../abstraction/Mesh.h"

#include <glm/glm.hpp>

class Renderer::Frustum;

class WaterSource
{

private:

	float			m_height;
	float			m_size = 10.F;

	glm::vec2		m_position;

	Renderer::Mesh	m_mesh;


public:

	//------------------------------------//

	WaterSource() 
		: m_height(0), m_position{0.f,0.f} {}

	WaterSource(float level, const glm::vec2& pos) 
		: m_height(level), m_position(pos) 
	{
		m_mesh = Renderer::createPlaneMesh(4);
	}

	WaterSource(const WaterSource& other)
		: m_height(other.m_height), m_position(other.m_position), m_size(other.m_size)
	{}

	WaterSource& operator=(const WaterSource& other) {
		m_height = other.m_height;
		m_position = other.m_position;
		m_size = other.m_size;		
	}

	//------------------------------------//

	void draw(const Renderer::Camera& camera) const {

		Renderer::renderMesh(
			{ m_position.x, m_height, m_position.y },
			{100, 1, 100},
			m_mesh,
			camera);
	}


	bool isOnFrustum(Renderer::Frustum& frustum) {

		return Renderer::Frustum::isOnFrustum(frustum, m_mesh.getBoundingBox());
	}

	//------------------------------------//

	float		getHeight()		const { return m_height; }
	float		getSize()		const { return m_size; }
	glm::vec2	getPosition()	const { return m_position; }

	void		setHeight(float level)				{ m_height = level; }
	void		setSize(float size)					{ m_size = size; }
	void		setPosition(const glm::vec2& pos)	{ m_position = pos; }


};

