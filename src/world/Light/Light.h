#pragma once

#include <glm/glm.hpp>


class Light {

	struct LightParam {

		glm::vec3 ambiant;
		glm::vec3 diffuse;
		glm::vec3 specular;

	};


private:

	glm::vec3	m_position;

	LightParam	m_params;

	bool		m_isOn;
	
	float		m_constant;
	float		m_linear;
	float		m_quadratic;


public:

	LightParam& getParams() { return m_params; }

	void setPosition(const glm::vec3& pos) { m_position = pos; }


	void setOn(bool isOn = true) {
		m_isOn = isOn;
	}




};