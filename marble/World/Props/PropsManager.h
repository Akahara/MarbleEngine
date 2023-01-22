#pragma once

#include <queue>
#include <vector>

#include "../../abstraction/Mesh.h"
#include "../../abstraction/Camera.h"

namespace World {

class PropsManager
{

private:

	struct Prop {

		std::shared_ptr< Renderer::Mesh>  mesh;
		glm::vec3 position = {0,0,0};
		glm::vec3 size = { 1,1,1 };
		std::string name = "N/A";

	};

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
	void render(const Renderer::Camera& camera);

	void onImGuiRender() ;

	//void feedFromFile(const )


};


}