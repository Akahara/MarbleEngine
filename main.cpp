#include <thread>
#include <chrono>

#include "src/vendor/imgui/imgui.h"
#include "src/vendor/imgui/imgui_impl_glfw.h"
#include "src/vendor/imgui/imgui_impl_opengl3.h"

#include "src/abstraction/Window.h"
#include "src/abstraction/Renderer.h"
#include "src/abstraction/Camera.h"
#include "src/abstraction/Cubemap.h"
#include "src/abstraction/Inputs.h"
#include "src/abstraction/TempRenderer.h"
#include "src/world/Player.h"
#include "src/world/TerrainGeneration/MapGenerator.h"
#include "src/World/TerrainGeneration/HeightMap.h"
#include "src/Sandbox/Scene.h"

inline long long nanoTime()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

class TestScene : public Scene {
private:
  Renderer::Camera m_Camera{ -1.0f, 1.0f, -1.0f, 1.0f };
  float            m_time = 0;
public:
  void Step(float delta) override
  {
    m_time += delta;
  }

  void OnRender() override
  {
    Renderer::Renderer::Clear();
    Renderer::Renderer::BeginBatch(m_Camera);
    for (float y = -1.0f; y < 1.0f; y += 0.025f) {
      for (float x = -1.0f; x < 1.0f; x += 0.025f) {
        glm::vec4 color = { (x + 1.0f) / 2.0f * (sin(m_time * 2) + 1),
                            (x + y) / 2.0f * (cos(m_time * 7) + 1),
                            (y + 1.0f) / 2.0f * (sin(m_time * 2) + 1),
                            1.0f };
        Renderer::Renderer::DrawQuad({ x,y, 0.0f }, { 0.02f, 0.02f }, color);
      }
    }
    Renderer::Renderer::EndBatch();
    Renderer::Renderer::Flush();
  }

  void OnImGuiRender() override {}
};

class Test3DScene : public Scene {
private:
  Renderer::Cubemap m_skybox;
  Player            m_player;
public:
  Test3DScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
  {
  }

  void Step(float delta) override
  {
    m_player.Step(delta);
  }

  void OnRender() override
  {
    Renderer::Renderer::Clear();
    Renderer::CubemapRenderer::DrawCubemap(m_skybox, m_player.GetCamera(), m_player.GetPosition());
    TempRenderer::RenderCube({ 1, 0, 0 }, glm::vec3{ 2.f, .05f, .05f }, { 0.f, 0.f, 1.f }, m_player.GetCamera().getViewProjectionMatrix()); // +x blue
    TempRenderer::RenderCube({ 0, 1, 0 }, glm::vec3{ .05f, 2.f, .05f }, { 1.f, 0.f, 0.f }, m_player.GetCamera().getViewProjectionMatrix()); // +y red
    TempRenderer::RenderCube({ 0, 0, 1 }, glm::vec3{ .05f, .05f, 2.f }, { 0.f, 1.f, 0.f }, m_player.GetCamera().getViewProjectionMatrix()); // +z green
    TempRenderer::RenderCube({ 0, 0, 0 }, glm::vec3{ 1.f,  1.f,  1.f }, { .9f, .9f, .9f }, m_player.GetCamera().getViewProjectionMatrix()); // unit cube
  }

  void OnImGuiRender() override {}
};

class TestTerrainScene : public Scene {
private:
  Renderer::Cubemap m_skybox;
  Player            m_player;
  HeightMap         m_heightmap;
  bool              m_playerIsFlying = true;
  int w = 200, h = 200;
  float scale = 27.6f;
  int o = 4;
  float p = 0.3f, l = 3.18f;
  int seed = 5;
  unsigned int id;
public:
  TestTerrainScene()
    : m_skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" }
  {
    float *noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);
    for (size_t i = 0; i < w * h; i++) // TODO remove and add an amplitude slider to GenerateNoiseMap
      noiseMap[i] *= 15.f;             // TODO also color the terrain differently based on that amplitude
    MapGenerator mapGen(w, h, scale, o, p, l, seed); // not sure of how the generator intervenes
    m_player.setPostion({ 0.f, 30.f, 0 });
    m_heightmap.setHeights(w, h, noiseMap);
    id = mapGen.GenerateTextureMap(w, h, noiseMap);
  }

  void Step(float delta) override
  {
    m_player.Step(delta);
    if (!m_playerIsFlying) {
      glm::vec3 pos = m_player.GetPosition();
      pos.y = m_heightmap.getHeightLerp(pos.x, pos.z) + 1.f;
      m_player.setPostion(pos);
      m_player.UpdateCamera();
    }
  }

  void OnRender() override
  {
    Renderer::CubemapRenderer::DrawCubemap(m_skybox, m_player.GetCamera(), m_player.GetPosition());
    // TODO FIX - currently RenderGrid takes a "quadsPerSide" parameter which should realy be "gridWidth" *and* "gridHeight"
    // and the scale parameter should be computed based on the grid size
    // and the mesh should be generated based on the heightmap
    float scale = w;
    int quadsPerSide = w - 1;
    TempRenderer::RenderCube({}, { 1.f, 1.f, 1.f }, { 1.f, 0.f, 1.f }, m_player.GetCamera().getViewProjectionMatrix());
    TempRenderer::RenderGrid({ 0, 0, 0 }, scale, quadsPerSide, { 1.f, 1.f, 1.f }, m_player.GetCamera().getViewProjectionMatrix(), id, m_heightmap.getBackingArray(), false);
  }

  void OnImGuiRender() override
  {
    if (ImGui::SliderInt("Width", &w, 10, 2000) + ImGui::SliderInt("Height", &h, 10, 2000) +
        ImGui::SliderFloat("Scale", &scale, 0, 50) + ImGui::SliderInt("Number of octaves", &o, 0, 10) +
        ImGui::SliderFloat("persistence", &p, 0, 1) + ImGui::SliderFloat("lacunarity", &l, 0, 50) +
        ImGui::SliderInt("seed", &seed, 0, 5)) {

      unsigned int nid;

      float *noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);
      m_heightmap.setHeights(w, h, noiseMap);

      // Texture stuff

      (glGenTextures(1, &nid));
      (glBindTexture(GL_TEXTURE_2D, nid));

      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
      (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

      if (noiseMap) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_FLOAT, noiseMap);
        glBindTexture(GL_TEXTURE_2D, 0);
      }

      id = nid;
    }

    ImGui::Checkbox("Fly", &m_playerIsFlying);
  }
};

int main()
{
	Window::createWindow(16*70, 9*70, "test");

    Window::setVisible(true);
    Window::setPosition(400, 100);
    Window::capFramerate();
    Inputs::ObserveInputs();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(Window::GetWindowHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

	Renderer::Renderer::Init();
    Renderer::CubemapRenderer::Init();
    TempRenderer::Init();
    SceneManager::Init();

    SceneManager::RegisterScene<TestScene>("Test");
    SceneManager::RegisterScene<Test3DScene>("Test3D");
    SceneManager::RegisterScene<TestTerrainScene>("Terrain");
    SceneManager::SwitchToScene(3);

    //===========================================================//

    unsigned int frames = 0;
    auto firstTime = nanoTime();
    auto lastSec = firstTime;

    while (!Window::shouldClose()) {

        Renderer::Renderer::Clear();
        auto nextTime = nanoTime();
        auto delta = nextTime - firstTime;
        float realDelta = delta / 1E9f;
        firstTime = nextTime;

        Window::pollUserEvents();
        Inputs::UpdateInputs();
        frames++;

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        
        SceneManager::Step(realDelta);
        SceneManager::OnRender();
        SceneManager::OnImGuiRender();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        Window::sendFrame();

        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            Window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }
    }

    SceneManager::Shutdown();

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

	return 0;
}
