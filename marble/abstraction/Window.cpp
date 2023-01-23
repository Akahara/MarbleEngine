#include "Window.h"

#include <iostream>
#include <vector>
#include <exception>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


using namespace Window::Inputs;

namespace Window {

static GLFWwindow *window = nullptr;
static unsigned int winWidth, winHeight;
static std::vector<InputHandler*> inputHandlers;

void GLAPIENTRY openglMessageCallback(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
  const char *sourceName;
  switch (source) {
  case GL_DEBUG_SOURCE_API:               sourceName = "API";               break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     sourceName = "WINDOW SYSTEM";     break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER:   sourceName = "SHADER COMPILER";   break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:       sourceName = "THIRD PARTY";       break;
  case GL_DEBUG_SOURCE_APPLICATION:       sourceName = "APPLICATION";       break;
  case GL_DEBUG_SOURCE_OTHER:             sourceName = "OTHER";             break;
  default:                                sourceName = "?";                 break;
  }

  const char *typeName;
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:               typeName = "ERROR";               break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeName = "DEPRECATED_BEHAVIOR"; break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeName = "UNDEFINED_BEHAVIOR";  break;
  case GL_DEBUG_TYPE_PORTABILITY:         typeName = "PORTABILITY";         break;
  case GL_DEBUG_TYPE_PERFORMANCE:         typeName = "PERFORMANCE";         break;
  case GL_DEBUG_TYPE_MARKER:              typeName = "MARKER";              break;
  case GL_DEBUG_TYPE_OTHER:               typeName = "OTHER";               break;
  default:                                typeName = "?";                   break;
  }

  const char *severityName;
  switch (severity) {
  case GL_DEBUG_SEVERITY_NOTIFICATION:    severityName = "NOTIFICATION";    break;
  case GL_DEBUG_SEVERITY_LOW:             severityName = "LOW";             break;
  case GL_DEBUG_SEVERITY_MEDIUM:          severityName = "MEDIUM";          break;
  case GL_DEBUG_SEVERITY_HIGH:            severityName = "HIGH";            break;
  default:                                severityName = "?";               break;
  }
  
  if ((type != GL_DEBUG_TYPE_PERFORMANCE) && (type != GL_DEBUG_TYPE_OTHER)) {
    std::cerr
      << "GL CALLBACK: " << sourceName << "\n"
      << " type     = " << typeName << "\n"
      << " severity = " << severityName << "\n"
      << " message  = " << message
      << std::endl;
  }
  
  if (
    (type != GL_DEBUG_TYPE_PERFORMANCE) &&
    (type != GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) &&
    (type != GL_DEBUG_TYPE_OTHER)
    ) {
    __debugbreak();
  }
}

void createWindow(unsigned int width, unsigned int height, const char *title)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_VISIBLE, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4); // Enable mutisampling (MSAA), must be done to framebuffers too

  window = glfwCreateWindow(width, height, title, NULL, NULL);
  winWidth = width;
  winHeight = height;

  if (window == NULL)
    throw std::exception("Failed to create GLFW window");

  glfwMakeContextCurrent(window);

  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
    for (InputHandler *handler : inputHandlers)
      handler->triggerKey(key, scancode, action, mods);
  });
  glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
    for (InputHandler *handler : inputHandlers)
      handler->triggerClick(button, action, mods);
  });
  glfwSetCursorPosCallback(window, [](GLFWwindow *window, double cursorX, double cursorY) {
    for (InputHandler* handler : inputHandlers)
      handler->triggerCursorMove(static_cast<int>(cursorX), winHeight - static_cast<int>(cursorY));
  });
  glfwSetScrollCallback(window, [](GLFWwindow *window, double scrollX, double scrollY) {
    for (InputHandler *handler : inputHandlers)
      handler->triggerScroll(scrollX, scrollY);
  });
  glfwSetCharCallback(window, [](GLFWwindow *window, unsigned int codepoint) {
    for (InputHandler *handler : inputHandlers)
      handler->triggerChar(codepoint);
  });
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    winWidth = width;
    winHeight = height;
  });

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    throw std::exception("Failed to initialize GLAD");

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(openglMessageCallback, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(5.f);
  glViewport(0, 0, width, height);
  glClearColor(0.f, 0.f, 0.0f, 1.0f);
}

void setVisible(bool visible)
{
  if (visible)
    glfwShowWindow(window);
  else
    glfwHideWindow(window);
}

bool shouldClose()
{
  return glfwWindowShouldClose(window);
}

void sendFrame()
{
  glfwSwapBuffers(window);
}

void pollUserEvents()
{
  glfwPollEvents();
}

void destroyWindow()
{
  inputHandlers.clear();
  glfwDestroyWindow(window);
  glfwTerminate();
}

unsigned int getWinWidth()
{
  return winWidth;
}

unsigned int getWinHeight()
{
  return winHeight;
}

void *getWindowHandle()
{
  return window;
}

void setAspectRatio(int w, int h)
{
  glfwSetWindowAspectRatio(window, w, h);
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
    glfwSetWindowMonitor(window, nullptr, 0, 0, winWidth, winHeight, GLFW_DONT_CARE);
  }
}

void setSize(unsigned int width, unsigned int height)
{
  glfwSetWindowSize(window, width, height);
}

void setPosition(int x, int y)
{
  glfwSetWindowPos(window, x, y);
}

void capFramerate()
{
  glfwSwapInterval(1);
}

void captureMouse(bool enabled)
{
  glfwSetInputMode(window, GLFW_CURSOR, enabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
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