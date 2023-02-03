#pragma once


#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "../UnifiedRenderer.h"
#include "Joint.h"
#include "Animator.h"
#include "Animation.h"


class AnimatedModel {


private:

	// skin
	Renderer::VertexArray m_VAO;
	Renderer::Texture m_texture;

	Renderer::Mesh m_mesh; // todo use this

	// skeleton
	Joint m_rootJoint;
	int  m_jointCount;

	Animator m_animator;

public:

	AnimatedModel(Renderer::VertexArray vao, Renderer::Texture texture, const Joint& joint, int jointCount)
		:
		m_VAO(std::move(vao)),
		m_texture(std::move(texture)),
		m_rootJoint(joint),
		m_jointCount(jointCount)
	{
		m_rootJoint.computeIBT(glm::mat4());
	}


	AnimatedModel(Renderer::Mesh&& mesh, const Joint& joint, int jointCount)
		:
		m_mesh(std::move(mesh)),
		m_rootJoint(joint),
		m_jointCount(jointCount)
	{
		m_rootJoint.computeIBT(glm::mat4());
	}

	AnimatedModel() {}
	//------------//

	void doAnimation(Animation& animation) {
		m_animator.doAnimation(&animation);
	}

	void step(float delta) {
		m_animator.step(delta);
	}

	
	const Joint& getRootJoint() const {
		return m_rootJoint;
	}

	 Joint& getRootJoint()  {
		return m_rootJoint;
	}

};