#include "window.h"

#include <iostream>
#include <vector>
#include <exception>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


using namespace window::inputs;

namespace window {

static GLFWwindow *window = nullptr;
static unsigned int winWidth, winHeight;
static std::vector<InputHandler*> inputHandlers;

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
  winWidth = width;
  winHeight = height;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  for (InputHandler *handler : inputHandlers)
    handler->triggerKey(key, scancode, action, mods);
}

void clickCallback(GLFWwindow *window, int button, int action, int mods)
{
  for (InputHandler *handler : inputHandlers)
    handler->triggerClick(button, action, mods);
}

void cursorMoveCallback(GLFWwindow *window, double cursorX, double cursorY)
{
  for (InputHandler *handler : inputHandlers)
    handler->triggerCursorMove(static_cast<int>(cursorX), winHeight - static_cast<int>(cursorY));
}

void scrollCallback(GLFWwindow *window, double scrollX, double scrollY)
{
  for (InputHandler *handler : inputHandlers)
    handler->triggerScroll(scrollX, scrollY);
}

void charCallback(GLFWwindow *window, unsigned int codepoint)
{
  for (InputHandler *handler : inputHandlers)
    handler->triggerChar(codepoint);
}

void GLAPIENTRY openglMessageCallback(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
  std::cerr
    << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") << "\n"
    << " type     = 0x" << std::hex << type << "\n"
    << " severity = 0x" << std::hex << severity << "\n"
    << " message  = " << message
    << std::dec << std::endl;
}

void createWindow(unsigned int width, unsigned int height, const char *title)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_VISIBLE, 0);
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(width, height, title, NULL, NULL);
  winWidth = width;
  winHeight = height;

  if (window == NULL)
    throw std::exception("Failed to create GLFW window");

  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetMouseButtonCallback(window, clickCallback);
  glfwSetCursorPosCallback(window, cursorMoveCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glfwSetCharCallback(window, charCallback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::exception("Failed to initialize GLAD");

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(openglMessageCallback, 0);
  glViewport(0, 0, width, height);
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);
  //glEnable(GL_MULTISAMPLE);
  //glLineWidth(.5f);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

unsigned int getWinWidth()
{
  return winWidth;
}

unsigned int getWinHeight()
{
  return winHeight;
}

void *getWindow()
{
  return window;
}

void setVisible(bool visible)
{
  if (visible)
    glfwShowWindow(window);
  else
    glfwHideWindow(window);
}

void setAspectRatio(int w, int h)
{
  glfwSetWindowAspectRatio(window, w, h);
}

void sendFrame()
{
  glfwSwapBuffers(window);
}

bool shouldClose()
{
  return glfwWindowShouldClose(window);
}

void pollUserEvents()
{
  glfwPollEvents();
}

void renameWindow(const char *title)
{
  glfwSetWindowTitle(window, title);
}

void setFullScreen(bool fullScreen)
{
  if (fullScreen) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
  } else {
    // TODO test the fullscreen->window transition
    glfwSetWindowMonitor(window, nullptr, 0, 0, winWidth, winHeight, GLFW_DONT_CARE);
  }
}

void setSize(unsigned int width, unsigned int height)
{
  glfwSetWindowSize(window, width, height);
}

void capFramerate()
{
  glfwSwapInterval(1);
}

void destroyWindow()
{
  glfwDestroyWindow(window);
  glfwTerminate();
  inputHandlers.clear();
}

void registerInputHandler(InputHandler *handler)
{
  inputHandlers.push_back(handler);
}

void unregisterInputHandler(InputHandler *handler)
{
  for (size_t i = 0; i < inputHandlers.size(); i++) {
    if (inputHandlers[i] == handler)
      inputHandlers.erase(inputHandlers.begin() + i);
    else
      i++;
  }
}

};