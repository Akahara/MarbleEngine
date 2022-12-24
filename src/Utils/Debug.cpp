#include "Debug.h"

#include <string>
#include <iostream>
#include <sstream>

#include <glad/glad.h>

#include "../vendor/imgui/imgui.h"
#include "../abstraction/UnifiedRenderer.h"


namespace DebugWindow {

static bool s_wireframeDisplay = false;
static bool s_renderAABBs = false;

void onImGuiRender()
{
  const auto &debugData = Renderer::getRendererDebugData();

  if (ImGui::Begin("Debug")) {
      ImGui::Text("Number of drawn vertices : %d\n", debugData.vertexCount);
      ImGui::Text("Number of drawn meshes : %d\n", debugData.meshCount);
      ImGui::Text("Number of debug lines : %d\n", debugData.debugLines);
      if (ImGui::Checkbox("Render as wireframe", &s_wireframeDisplay)) {
        glPolygonMode(GL_FRONT_AND_BACK, s_wireframeDisplay ? GL_LINE : GL_FILL);
      }
      ImGui::Checkbox("Render AABBS", &s_renderAABBs);
  }

  ImGui::End();
  Renderer::clearDebugData();
}

bool renderAABB() {
  return s_renderAABBs;
}

}