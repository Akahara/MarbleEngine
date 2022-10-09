#pragma once

#include <assert.h>

#include "../abstraction/Shader.h"

namespace DebugWindow {

void OnImGuiRender();

}

class TestUniform {
private:
  Renderer::Shader *m_shader;
  float             m_value;
  const char       *m_name;
  float             m_speed;
public:
  TestUniform(Renderer::Shader &shader, const char *name, float defaultValue=0.f, float speed=.1f)
    : m_shader(&shader), m_name(name), m_value(defaultValue), m_speed(speed) { }

  void RenderImGui();
};