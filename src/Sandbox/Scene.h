#pragma once

#include <string>
#include <functional>

#include "../vendor/imgui/imgui.h"

class Scene {
public:
  virtual ~Scene() = default;

  virtual void Step(float delta) = 0; // OnUpdate
  virtual void OnRender() = 0;
  virtual void OnImGuiRender() = 0;
};

//typedef Scene *(*SceneProvider)();
typedef std::function<Scene *()> SceneProvider;

namespace SceneManager {

void Init();
void Shutdown();

void Step(float delta);
void OnRender();
void OnImGuiRender();

void RegisterScene(const std::string &name, SceneProvider provider);
template<class T>
void RegisterScene(const std::string &name) { RegisterScene(name, []() { return new T; }); }

void SwitchToScene(size_t index);

}