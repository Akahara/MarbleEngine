#pragma once

#include "KeyFrame.h"
#include <vector>

class Animation {

private:

	float m_length;
	std::vector<KeyFrame> m_keyFrames;


public:

	Animation(float lengthInSeconds, const std::vector<KeyFrame>& frames) : m_length(lengthInSeconds), m_keyFrames(frames)
	{}

	Animation() {}

	float getLength() const { return m_length; }

	const std::vector<KeyFrame>& getKeyFrames() const { return m_keyFrames; }

};