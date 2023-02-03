#pragma once


#include "JointTransform.h"
#include <unordered_map>

class KeyFrame {

private:


	float m_timeStamp;
	std::unordered_map<std::string, JointTransform> m_pose;


public:

	KeyFrame(float timeStamp, const std::unordered_map<std::string, JointTransform>& pose) : m_timeStamp(timeStamp), m_pose(pose)
	{}

	float getTimeStamp() const { return m_timeStamp; }
	const std::unordered_map<std::string, JointTransform> & getPoses() const { return m_pose; }

};