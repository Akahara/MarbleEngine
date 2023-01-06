#pragma once

#include "WaterSource.h"

#include <vector>

#include "../../abstraction/Texture.h"
#include "../../abstraction/UnifiedRenderer.h"
#include "../../abstraction/FrameBufferObject.h"
#include "../../abstraction/Camera.h"
#include "../../abstraction/Window.h"


class WaterRenderer
{
private:
  static constexpr float WAVE_SPEED = 0.05f;

  Renderer::FrameBufferObject m_waterRefractionFbo;
  Renderer::FrameBufferObject m_waterReflectionFbo;

  Renderer::Texture m_refractionTexture{ Window::getWinWidth(), Window::getWinHeight() }; // what u see under water
  Renderer::Texture m_reflectionTexture{ Window::getWinWidth(), Window::getWinHeight() }; // what is reflected

  Renderer::Texture m_dudvTexture = Renderer::Texture("res/textures/dudvWater.png"); //distortion
  Renderer::Texture m_normalTexture = Renderer::Texture("res/textures/waterNormal.png"); //normal

  Renderer::Shader m_waterShader = Renderer::loadShaderFromFiles("res/shaders/standard.vs", "res/shaders/water.fs");
  Renderer::Texture m_depthTexture = Renderer::Texture::createDepthTexture( Window::getWinWidth(), Window::getWinHeight() );
  // somehow the depth texture of the water is broken

  float m_moveFactor = 0;

public:

	WaterRenderer();

  void updateMoveFactor(float deltaTime);

  // -------- Reflection

  void setupCameraForReflection(Renderer::Camera &cam, const WaterSource &source);
  void undoSetupCameraForReflection(Renderer::Camera &cam, const WaterSource &source);

  void bindReflectionBuffer();

  //--------- Refraction

  void bindRefractionBuffer();

  void unbind();

  /* TO BE CALLED AFTER BINDING THE VFX PIPELINE AND THE SCENE HAS BEEN RENDERED IN THE WATER FBO */
  void onRenderWater(const std::vector<WaterSource *> &waterSources, const Renderer::Camera &camera);

  void drawWaterSource(const WaterSource &source, const Renderer::Camera &camera);

  // Debug method, writes the rendered reflection and refraction textures to files
  void showDebugTextures();
};

