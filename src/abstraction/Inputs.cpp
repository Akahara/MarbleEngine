#include "Inputs.h"

#include <set>

#include <GLFW/glfw3.h>

#include "Window.h"

static bool escaped = false;
namespace WI = Window::Inputs;

namespace Inputs {

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

    if (key == GLFW_KEY_E && action != WI::ACTION_RELEASE) {

        escaped = !escaped;
        Window::captureMouse(escaped);

    }
  }

  void triggerCursorMove(int x, int y) override
  {
    s_cursorPosition = { x, y };
  }
};

void ObserveInputs()
{
  Window::registerInputHandler(new InputsObserver);
  Window::captureMouse();
}

void UpdateInputs()
{
  s_cursorPositionDelta = s_cursorPosition - s_previousCursorPosition;
  s_previousCursorPosition = s_cursorPosition;
}

bool IsKeyPressed(int keycode)
{
  return s_pressedKeys.contains(keycode);
}

glm::vec2 GetMousePosition()
{
  return s_cursorPosition;
}

glm::vec2 GetMouseDelta()
{
  return s_cursorPositionDelta;
}

glm::vec2 GetInputRange()
{
  return { Window::getWinWidth(), Window::getWinHeight() };
}

}