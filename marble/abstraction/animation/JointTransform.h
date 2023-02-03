#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class JointTransform {

private:

	glm::vec3 m_position;
	glm::quat m_rotation;


public:

	JointTransform(const glm::vec3& position, const glm::quat& rotation) : m_position(position), m_rotation(rotation) {}


	glm::mat4 getLocalTransform() {

		glm::mat4 matrix(1.0F);
		matrix = glm::translate(matrix, m_position);
		matrix = matrix * glm::mat4_cast(m_rotation);
		return matrix;

	}

	static JointTransform interpolate(const JointTransform& a, const JointTransform& b, float x) {

		glm::vec3 pos = glm::mix(a.m_position, b.m_position, x);
		glm::quat rot = glm::slerp(a.m_rotation, b.m_rotation, x);

		return JointTransform(pos, rot);

	}

};