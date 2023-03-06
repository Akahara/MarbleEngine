#pragma once

#include <glm/glm.hpp>
#include <unordered_map>

#include "../../abstraction/UnifiedRenderer.h"

// This file has to change.
// Change distance to fall-off, don't stick to such a small sample of fall-offs, add an intensity parameter for HDR, support more than 12 lights...

static std::unordered_map<float, glm::vec3> s_mapDistValues =
{
	//https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation

	//distance	   constant        linear			 quad
	{3250.f			,{ 1.0		, 0.0014			, 0.000007	}},
	{600.f			,{ 1.0		, 0.007				, 0.0002	}},
	{325.f			,{ 1.0		, 0.014				, 0.0007	}},
	{200.f			,{ 1.0		, 0.022				, 0.0019	}},
	{160.f			,{ 1.0		, 0.027				, 0.0028	}},
	{100.f			,{ 1.0		, 0.045				, 0.0075	}},
	{65.f			,{ 1.0		, 0.07				, 0.017		}},
	{50.f			,{ 1.0		, 0.09				, 0.032		}},
	{32.f			,{ 1.0		, 0.14				, 0.07		}},
	{20.f			,{ 1.0		, 0.22				, 0.20		}},
	{13.f			,{ 1.0		, 0.35				, 0.44		}},
	{7.f			,{ 1.0		, 0.7				, 1.8		}}

};

static std::vector<float> s_keys = {
	3250.f,
	600.f,
	325.f,
	200.f,
	160.f,
	100.f,
	65.f,
	50.f,
	32.f,
	20.f,
	13.f,
	7.f
};


template <typename T>
T closest(std::vector<T> const& vec, T value)
{
	T closest = INFINITY;
	T res = vec.at(0);
	for (const T& val : vec) {
		if ( std::abs(value - val) < closest) {
			closest = std::abs(value - val);
			res = val;
		}
	}

	return res;
}

class Light {
public:
	struct LightParam {
		glm::vec3 ambiant;
		glm::vec3 diffuse;
		glm::vec3 specular;
	};

private:
	struct LightCoefs {
		float constant  = 0;
		float linear    = 0;
		float quadratic = 0;

		LightCoefs() {}
		LightCoefs(const glm::vec3& val) {
			constant  = val.x;
			linear    = val.y;
			quadratic = val.z;
		}
	};

	glm::vec3  m_position;

	LightParam m_params;
	LightCoefs m_coefficients;

	float      m_distance;
	bool       m_isOn;

	void computeLightCoefs(float distance)
	{
		float closestDistanceValue = closest<float>(s_keys, distance);
		m_coefficients = s_mapDistValues.at(closestDistanceValue);
		m_distance = closestDistanceValue;
	}

public:
	Light() : m_position(0)
			, m_params{ {0,0,0},{0,0,0},{0,0,0} }
			, m_isOn(false)
			, m_distance(7.0f)
	{
		computeLightCoefs(7.0f);
	}

	Light(const glm::vec3& position,
		const LightParam& params,
		float distance = 30,
		bool isOn = true) 
	{
		m_params = params;
		m_position = position;
		m_isOn = isOn;
		computeLightCoefs(distance);
	}

	const LightParam& getParams() const { return m_params; }
	LightCoefs getCoefs() const { return m_coefficients; }
	glm::vec3 getPosition() const { return m_position; }

	void setPosition(const glm::vec3& pos) { m_position = pos; }
	void setDistance(float distance) { computeLightCoefs(distance); }
	float getDistance() const { return m_distance; }
	bool isOn() const { return m_isOn; }
	void setOn(bool isOn = true) { m_isOn = isOn; }
};
