#pragma once

/* Immediate wrapper of the GL concept */
/*
* A single window may exist at any given time.
* 
* Inputs can be handled using #registerInputHandler with an InputHandler
* instance that overrides one or more methods.
*/
namespace Window {

namespace Inputs {

typedef int action_t;
typedef int button_t;

constexpr action_t ACTION_RELEASE = 0;
constexpr action_t ACTION_PRESS   = 1;
constexpr action_t ACTION_REPEAT  = 2;
constexpr button_t BUTTON_LEFT    = 0;
constexpr button_t BUTTON_RIGHT   = 1;

class InputHandler {
public:
  virtual void triggerKey(int key, int scancode, action_t action, int mods) {}
  virtual void triggerClick(button_t button, action_t action, int mods) {}
  virtual void triggerCursorMove(int x, int y) {}
  virtual void triggerScroll(double xDelta, double yDelta) {}
  virtual void triggerChar(unsigned int codepoint) {}
};

}

void createWindow(unsigned int width, unsigned int height, const char *title);
void setVisible(bool visible = true);
bool shouldClose();
void sendFrame();
void pollUserEvents();
void destroyWindow();

// Unsafe, returns a GLFWwindow*
void *getWindowHandle();

unsigned int getWinWidth();
unsigned int getWinHeight();
void setAspectRatio(int w, int h);
void setSize(unsigned int w, unsigned int h);
void setPosition(int x, int y);
void renameWindow(const char *title);
void setFullScreen(bool fullScreen);
void capFramerate();
void captureMouse(bool enable=true);

void registerInputHandler(Inputs::InputHandler *handler);
void unregisterInputHandler(Inputs::InputHandler *handler);

};