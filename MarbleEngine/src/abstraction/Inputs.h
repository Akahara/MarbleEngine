#pragma once

#include "../../Libraries/include/glm/vec2.hpp"

namespace Inputs {

void observeInputs();
void updateInputs();

bool isKeyPressed(int keycode);
glm::vec2 getMousePosition();
glm::vec2 getMouseDelta();
glm::vec2 getInputRange(); // returns the window size, GetMousePosition and GetMouseDelta are in that range

}