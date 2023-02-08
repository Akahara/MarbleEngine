#include "Inputs.h"

#include <cstdlib>
#include <set>

#include <GLFW/glfw3.h>

#include "Window.h"

namespace WI = Window::Inputs;

namespace Inputs {

static int s_cursorLockFrames = 0;
static std::set<int> s_pressedKeys;
static glm::vec2 s_previousCursorPosition;
static glm::vec2 s_cursorPosition;
static glm::vec2 s_cursorPositionDelta;

class InputsObserver : public WI::InputHandler {
public:
  void triggerKey(int key, int scancode, WI::action_t action, int mods) override {
    if (action == WI::ACTION_PRESS)
      s_pressedKeys.insert(key);
    else if (action == WI::ACTION_RELEASE)
      s_pressedKeys.erase(key);
    
    if (key == GLFW_KEY_ESCAPE)
      exit(0);

    if (key == GLFW_KEY_E && action == WI::ACTION_PRESS) {
      s_cursorLockFrames = s_cursorLockFrames >= 0 ? -1 : 0;
      Window::captureMouse(s_cursorLockFrames >= 0);
    }
  }

  void triggerCursorMove(int x, int y) override
  {
    s_cursorPosition = { x, y };
  }

};

void observeInputs()
{
  Window::registerInputHandler(new InputsObserver);
  Window::captureMouse();
}

void updateInputs()
{
  s_cursorPositionDelta = s_cursorLockFrames > 1 ? s_cursorPosition - s_previousCursorPosition : glm::vec2{};
  if (s_cursorLockFrames >= 0) s_cursorLockFrames++;
  s_previousCursorPosition = s_cursorPosition;
}

bool isKeyPressed(int keycode)
{
  return s_pressedKeys.contains(keycode);
}

glm::vec2 getMousePosition()
{
  return s_cursorPosition;
}

glm::vec2 getMouseDelta()
{
  return s_cursorPositionDelta;
}

glm::vec2 getInputRange()
{
  return { Window::getWinWidth(), Window::getWinHeight() };
}

}