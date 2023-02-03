#pragma once


#include "AnimatedModel.h"
#include "Animation.h"

#include <unordered_map>
#include <glm/glm.hpp>

class Animator {

private:

	AnimatedModel m_entity;
	Animation* m_currentAnimation;

	float m_animationTime;


public:

	Animator(AnimatedModel&& entity) : m_entity(std::move(entity)), m_animationTime(0), m_currentAnimation(nullptr) {}
	Animator(): m_animationTime(0), m_currentAnimation(nullptr) {}

	void doAnimation(Animation* animation) {

		m_animationTime = 0;
		m_currentAnimation = animation;
	}

	void step(float delta) {

		if (!m_currentAnimation) return;

		increaseAnimationTime(delta);
		std::unordered_map<std::string, glm::mat4> currentPose = calculateCurrentAnimationPose();
		applyPoseToJoints(currentPose, m_entity.getRootJoint(), glm::mat4(1.0f));



	}

	void increaseAnimationTime(float delta) {
		m_animationTime += delta;

		if (m_animationTime > m_currentAnimation->getLength()) 
		{
			m_animationTime = fmod(m_animationTime, m_currentAnimation->getLength());
		}

	}

	std::unordered_map<std::string, glm::mat4> calculateCurrentAnimationPose() {

		std::vector<KeyFrame> frames = getPreviousAndNextFrames();
		float progression = calculateProgression(frames[0], frames[1]);
		return interpolatePoses(frames[0], frames[1], progression);

	}

	std::vector<KeyFrame> getPreviousAndNextFrames() {

		std::vector<KeyFrame> allframes = m_currentAnimation->getKeyFrames();

		KeyFrame prev = allframes[0];
		KeyFrame next = allframes[0];

		for (int i = 1; i < allframes.size(); i++) {
			next = allframes[i];
			if (allframes[i].getTimeStamp() > m_animationTime) {
				break;
			}
			prev = allframes[i];


		}

		return {prev, next};
	}


	void applyPoseToJoints(const std::unordered_map<std::string, glm::mat4>& currentPos, Joint& joint, const glm::mat4& parentTransform) 
	{
		glm::mat4 currentLocalTransform = currentPos.at(joint.getName());
		glm::mat4 currentTransform = parentTransform * currentLocalTransform;

		for (Joint& child : joint.getChildren()) {
			applyPoseToJoints(currentPos, child, currentTransform);
		}

		currentTransform = currentTransform * joint.getIBT();
		joint.setAnimationTransform(currentTransform);


	}

	float calculateProgression(const KeyFrame& a, const KeyFrame& b) {

		float totaltime = b.getTimeStamp() - a.getTimeStamp();
		float curr = m_animationTime - a.getTimeStamp();
		return curr / totaltime;
	}

	std::unordered_map<std::string, glm::mat4> interpolatePoses(const KeyFrame& a, const KeyFrame& b, float x) {


		std::unordered_map<std::string, glm::mat4> currentPos;

		for (const auto& [name, pose] : a.getPoses()) {
			JointTransform previousTransform = a.getPoses().at(name);
			JointTransform nextTransform = b.getPoses().at(name);
			JointTransform currentTransform = JointTransform::interpolate(previousTransform, nextTransform, x);

			currentPos[name] = currentTransform.getLocalTransform();


		}

		return currentPos;

		
	}


};