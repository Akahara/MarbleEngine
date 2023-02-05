#pragma once

#include <vector>

#include "../../abstraction/Mesh.h"

namespace World {

	struct Prop {

		std::shared_ptr<Renderer::Mesh> mesh;
		glm::vec3 position = { 0,0,0 };
		glm::vec3 size = { 1,1,1 };
		std::string name = "N/A";


	};

};