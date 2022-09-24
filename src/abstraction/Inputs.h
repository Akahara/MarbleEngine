#pragma once

#include <glm/vec2.hpp>

namespace Inputs {

void ObserveInputs();
void UpdateInputs();

bool IsKeyPressed(int keycode);
glm::vec2 GetMousePosition();
glm::vec2 GetMouseDelta();
glm::vec2 GetInputRange(); // returns the window size, GetMousePosition and GetMouseDelta are in that range

}