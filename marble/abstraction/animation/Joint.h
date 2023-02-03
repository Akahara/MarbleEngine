#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>


class Joint {

private:

	int m_id;
	std::string m_name;

	std::vector<Joint> m_children;

	glm::mat4 m_animatedTransform; // moves join,t from original position (tpos) to posed

	glm::mat4 m_localBindTransform; // local = relation to parent, bind = no animation
	glm::mat4 m_inverseBindTransform;



public:

	Joint(int index, const std::string& name, const glm::mat4& blT) 
		: m_id(index),
		m_name(name),
		m_localBindTransform(blT),
		m_inverseBindTransform(glm::mat4(1.0f)),
		m_animatedTransform(glm::mat4(1.0f))
	{}

	Joint() {}

	void addChild(const Joint& child) {
		m_children.push_back(child);
	}


	void setAnimationTransform(const glm::mat4& animationTransform) {
		m_animatedTransform = animationTransform;
	}

	glm::mat4 getAnimationTransform() const {
		return m_animatedTransform;
	}

	glm::mat4 getIBT() const {
		return m_inverseBindTransform;
	}

	std::vector<Joint>& getChildren() { return m_children; }

	std::string getName() const { return m_name; }

	void computeIBT(const glm::mat4& parentBindTransform) const {
		glm::mat4 bt = parentBindTransform * m_localBindTransform;
		bt = glm::inverse(bt);

		for (const Joint& child : m_children) {
			child.computeIBT(bt);
		}
	}



};