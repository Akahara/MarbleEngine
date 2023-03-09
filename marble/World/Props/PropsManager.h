#pragma once

#include <queue>
#include <vector>

#include "../../abstraction/Mesh.h"
#include "../../abstraction/Camera.h"

#include "Prop.h"

namespace World {

class PropsManager
{

private:

	std::vector<Prop> m_props;
	std::queue<Prop> m_toRender;


	void computeToBeRendered(const Renderer::Frustum& viewFrustum); // this could be done in parallel i guess

public:

	PropsManager()
	{

	}

	void clear();
	void feed(const std::shared_ptr< Renderer::Mesh>& mesh, 
		const glm::vec3& position = {0,0,0},
		const glm::vec3& size = {1,1,1});

	void feed(Prop prop);
	
	void render(const Renderer::Camera& camera);

	void onImGuiRender() ;


};


}