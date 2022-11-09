#pragma once

#include <string>
#include <functional>

#include "../Utils/Debug.h"
#include "../vendor/imgui/imgui.h"

class Scene {
public:
  virtual ~Scene() = default;

  virtual void step(float delta) = 0; // OnUpdate
  virtual void onRender() = 0;
  virtual void onImGuiRender() = 0;
};

//typedef Scene *(*SceneProvider)();
typedef std::function<Scene *()> SceneProvider;

namespace SceneManager {

void init();
void shutdown();

void step(float delta);
void onRender();
void onImGuiRender();

void registerScene(const std::string &name, SceneProvider provider);
template<class T>
void registerScene(const std::string &name) { registerScene(name, []() { return new T; }); }

void switchToScene(size_t index);



}