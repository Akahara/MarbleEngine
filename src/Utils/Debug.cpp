#include "Debug.h"

#include <glad/glad.h>
#include <string>
#include <iostream>
#include <sstream>
#include "../vendor/imgui/imgui.h"
#include "../abstraction/UnifiedRenderer.h"



namespace DebugWindow {

static bool s_wireframeDisplay = false;
void onImGuiRender()
{
    

  if (ImGui::Begin("Debug")) {

    if (ImGui::Checkbox("Render as wireframe", &s_wireframeDisplay)) {
      glPolygonMode(GL_FRONT_AND_BACK, s_wireframeDisplay ? GL_LINE : GL_FILL);
    }
  }

  ImGui::End();
}

}