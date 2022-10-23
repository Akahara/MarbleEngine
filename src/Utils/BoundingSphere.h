#pragma once

#include "BoundingVolume.h"

class BoundingSphere : public BoundingVolume {

private:
	glm::vec3	m_center;
	float		m_radius;
public:

	BoundingSphere() : 
		m_center{},
		m_radius{}
	{}

	BoundingSphere(const glm::vec3& center, float radius) :
		m_center(center),
		m_radius(radius)
	{}

	glm::vec3 getCenter() const { return m_center; }
	float getRadius() const { return m_radius; }

	void setCenter(const glm::vec3& center) {

		m_center = center;

	}

	void setRadius(float radius) {

		m_radius = radius;
	}

	bool isInBounds(const glm::vec3& point) const
	{
		return
			(glm::distance(point, m_center) < glm::distance(m_center, m_center + glm::vec3(m_radius)));
	}


};