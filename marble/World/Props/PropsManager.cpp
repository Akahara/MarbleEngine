#include "PropsManager.h"

#include <glm/gtc/type_ptr.hpp>

#include "../../abstraction/UnifiedRenderer.h"
#include "../../vendor/imgui/imgui.h"
#include "../../Utils/Debug.h"

namespace World {

void PropsManager::feed(const std::shared_ptr<Renderer::Mesh>& mesh)
{
  m_props.emplace_back(mesh);
}

void PropsManager::render(const Renderer::Camera& camera)
{
  Renderer::Frustum frustum = Renderer::Frustum::createFrustumFromCamera(camera);

  for(const std::shared_ptr<Renderer::Mesh> &prop : m_props) {
	AABB boundingBox = prop->getBoundingBox();
	if (frustum.isOnFrustum(boundingBox)) {
	  Renderer::renderMesh(camera, *prop);
	}
	if (DebugWindow::renderAABB()) {
	  Renderer::renderAABBDebugOutline(camera, boundingBox);
	}
  }
}

void PropsManager::onImGuiRender()
{
  if (ImGui::Begin("Scene props")) {
	for (unsigned int i = 0; i < m_props.size(); i ++) {
	  Renderer::Mesh& p = *m_props[i];
	  ImGui::PushID(i);
	  if (ImGui::CollapsingHeader(("Prop " + std::to_string(i)).c_str())) {
		ImGui::DragFloat3("Position", glm::value_ptr(p.getTransform().position), 1.F);
		ImGui::DragFloat3("Size", glm::value_ptr(p.getTransform().scale), 0.25F);
	  }
	  ImGui::PopID();
	}
  }
  ImGui::End();
}

void PropsManager::clear()
{
  m_props.clear();
}

} // !namespace World