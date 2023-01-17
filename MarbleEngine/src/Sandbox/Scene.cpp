#include "Scene.h"

#include <vector>

#include "../vendor/imgui/imgui.h"

#include "../abstraction/UnifiedRenderer.h"

class EmptyScene : public Scene {
public:
  void step(float delta) override {}
  void onRender() override { Renderer::clear(); }
  void onImGuiRender() override {}
  virtual Renderer::Camera& getCamera() override { throw std::exception("No camera has been set up for this scene"); }
};

namespace SceneManager {

static std::vector<std::pair<std::string, SceneProvider>> s_availableScenes;
static size_t s_activeSceneIndex = -1;
static Scene *s_activeScene = nullptr;

void init()
{
  registerScene<EmptyScene>("Empty");
  s_activeScene = s_availableScenes[0].second();
  s_activeSceneIndex = 0;
}

void shutdown()
{
  delete s_activeScene;
}

void switchToScene(size_t sceneIndex)
{
  // restore the default mesh shader for scenes that do not overwrite it
  // this also has the effect of reseting uniforms
  Renderer::rebuildStandardMeshShader(Renderer::ShaderFactory()
    .prefix("res/shaders/")
    .addFileVertex("standard.vs")
    .prefix("mesh_parts/")
    .addFileFragment("base.fs")
    .addFileFragment("color_terrain.fs")
    .addFileFragment("lights_none.fs")
    .addFileFragment("final_fog.fs")
    .addFileFragment("shadows_normal.fs")
    .addFileFragment("normal_none.fs"));

  delete s_activeScene;
  auto &[name, provider] = s_availableScenes[sceneIndex];
  s_activeScene = provider();
  s_activeSceneIndex = sceneIndex;
}

void step(float delta)
{
  s_activeScene->step(delta);
}

void onRender()
{
  s_activeScene->onRender();
}

void onImGuiRender()
{
  s_activeScene->onImGuiRender();

  ImGui::Begin("Scenes");
  for (size_t i = 0; i < s_availableScenes.size(); i++) {
    auto &[name, provider] = s_availableScenes[i];
    if (ImGui::Button(name.c_str()))
      switchToScene(i);
    if (s_activeSceneIndex == i) {
      ImGui::SameLine();
      ImGui::Text("< active");
    }
  }
  ImGui::End();
}

void registerScene(const std::string &name, SceneProvider provider)
{
  s_availableScenes.push_back(std::make_pair(name, provider));
}

}