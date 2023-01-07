#pragma once

#include <string>
#include <functional>

#include "../Utils/Debug.h"
#include "../vendor/imgui/imgui.h"
#include "../abstraction/Camera.h"

// Scene#getCamera is abstract, use these macros to quickly implement it
#define CAMERA_IS_PLAYER(player) virtual Renderer::Camera& getCamera() override { return player.getCamera(); }
#define CAMERA_NOT_DEFINED() virtual Renderer::Camera& getCamera() override { throw std::exception("No camera has been set up for this scene"); }

/**
* The scene class is the main development "unit", a scene is created
* and run until another takes its place.
* 
* The normal lifetime of a scene is:
* creation
*   resources loading (textures, meshes, terrain)
* cycle
*   physics update
*   display
* finalization
*   resources destruction
*/
class Scene {
public:
  virtual ~Scene() = default;

  virtual void step(float delta) = 0; // equivalent to OnUpdate in game engines (eg. Unity)
  virtual void onRender() = 0;
  virtual void onImGuiRender() = 0;

  virtual Renderer::Camera& getCamera() = 0;
};

typedef std::function<Scene *()> SceneProvider;

/*
* The scene manager is a utility that allows to easily start a scene
* and switch to another. It also shows a GUI to switch at runtime.
*/
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