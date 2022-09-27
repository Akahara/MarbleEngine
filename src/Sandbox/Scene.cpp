#include "Scene.h"

#include <vector>

#include "../vendor/imgui/imgui.h"

#include "../abstraction/Renderer.h"

class EmptyScene : public Scene {
public:
  void Step(float delta) override {}
  void OnRender() override {
    Renderer::Renderer::Clear();
  }
  void OnImGuiRender() override {}
};

namespace SceneManager {

static std::vector<std::pair<std::string, SceneProvider>> s_availableScenes;
static size_t s_activeSceneIndex = -1;
static Scene *s_activeScene = nullptr;

void Init()
{
  RegisterScene<EmptyScene>("Empty");
  s_activeScene = s_availableScenes[0].second();
  s_activeSceneIndex = 0;
}

void Shutdown()
{
  delete s_activeScene;
}

void SwitchToScene(size_t sceneIndex)
{
  delete s_activeScene;
  auto &[name, provider] = s_availableScenes[sceneIndex];
  s_activeScene = provider();
  s_activeSceneIndex = sceneIndex;
}

void Step(float delta)
{
  s_activeScene->Step(delta);
}

void OnRender()
{
  s_activeScene->OnRender();
}

void OnImGuiRender()
{
  s_activeScene->OnImGuiRender();

  ImGui::Begin("Scenes");
  for (size_t i = 0; i < s_availableScenes.size(); i++) {
    auto &[name, provider] = s_availableScenes[i];
    if (ImGui::Button(name.c_str()))
      SwitchToScene(i);
    if (s_activeSceneIndex == i) {
      ImGui::SameLine();
      ImGui::Text("< active");
    }
  }
  ImGui::End();
}

void RegisterScene(const std::string &name, SceneProvider provider)
{
  s_availableScenes.push_back(std::make_pair(name, provider));
}

}