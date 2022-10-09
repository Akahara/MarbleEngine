#include "Debug.h"

#include <glad/glad.h>

#include "../vendor/imgui/imgui.h"

namespace DebugWindow {

static bool s_wireframeDisplay = false;

void OnImGuiRender()
{
  if (ImGui::Begin("Debug")) {
    if (ImGui::Checkbox("Render as wireframe", &s_wireframeDisplay)) {
      glPolygonMode(GL_FRONT_AND_BACK, s_wireframeDisplay ? GL_LINE : GL_FILL);
    }
  }

  ImGui::End();
}

}

void TestUniform::RenderImGui()
{
  if (ImGui::DragFloat(m_name, &m_value, m_speed)) {
    m_shader->Bind();
    m_shader->SetUniform1f(m_name, m_value);
  }
}