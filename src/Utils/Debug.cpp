#include "Debug.h"

#include <glad/glad.h>
#include <string>
#include <iostream>
#include <sstream>
#include "../vendor/imgui/imgui.h"
#include "../abstraction/UnifiedRenderer.h"



namespace DebugWindow {

static bool s_wireframeDisplay = false;
bool aabbRender = false;

void onImGuiRender()
{


    const auto debug_data = Renderer::getRendererDebugData();
    

  if (ImGui::Begin("Debug")) {

      ImGui::Text("Number of drawn vertices : %d\n", debug_data.vertexCount);
      ImGui::Text("Number of drawn meshes : %d\n", debug_data.meshCount);
      ImGui::Text("Number of debug lines : %d\n", debug_data.debugLines);
    if (ImGui::Checkbox("Render as wireframe", &s_wireframeDisplay)) {
      glPolygonMode(GL_FRONT_AND_BACK, s_wireframeDisplay ? GL_LINE : GL_FILL);
    }
    (ImGui::Checkbox("Render AABBS", &aabbRender));


  }

  ImGui::End();
  Renderer::clearDebugData();
}

bool renderAABB() { return aabbRender; }

}