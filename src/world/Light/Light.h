#pragma once

#include <glm/glm.hpp>


class Light {

	struct LightParam {

		glm::vec3 ambiant;
		glm::vec3 diffuse;
		glm::vec3 specular;

	};

	struct LightCoefs {


		float	constant	= 0;
		float	linear		= 0;
		float	quadratic	= 0;

	};


private:

	glm::vec3	m_position;

	LightParam	m_params;
	LightCoefs m_coefficients;

	bool		m_isOn;
	


public:

	Light() : m_position(0)
			, m_params{ {0,0,0},{0,0,0},{0,0,0} }
			, m_isOn(false)
	{}

	Light(const glm::vec3& position,
		const glm::vec3& ambiant,
		const glm::vec3& diffuse,
		const glm::vec3& specular,
		bool isOn = true) {

		m_params = { ambiant, diffuse, specular };
		m_position = position;
		m_isOn = isOn;
		computeLightCoefs();

	}


	void computeLightCoefs(); // Look-up table based on distance



	LightParam& getParams()			{ return m_params; }
	LightCoefs& getCoefs()			{ return m_coefficients; }
	glm::vec3 getPosition() const	{ return m_position; }
	bool isOn() const				{ return m_isOn; }

	void setPosition(const glm::vec3& pos) { m_position = pos; }


	void setOn(bool isOn = true) {
		m_isOn = isOn;
	}




};