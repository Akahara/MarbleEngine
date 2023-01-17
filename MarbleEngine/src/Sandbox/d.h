#pragma once

#include <iostream>

#include "../../Libraries/include/glm/glm.hpp"

class objet {

	

public:

	glm::vec2 m = {0,0};

	void test() {
		std::cout << "dependency" << std::endl;
	}

};