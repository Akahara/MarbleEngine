
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
#include "src/Sandbox/Scene.h"
#include "src/abstraction/TempRenderer.h"

#include "src/world/Player.h"

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

int main()
{
	Window::createWindow(813, 546, "test");

    Window::setVisible(true);
    Window::setPosition(400, 100);
    Window::capFramerate();
    Inputs::ObserveInputs();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(Window::GetWindowHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

	Renderer::Renderer::Init();
    SceneManager::Init();

    SceneManager::RegisterScene<TestScene>("Test");

    //===========================================================//

	Renderer::Renderer::Init();
    Renderer::CubemapRenderer::Init();
    TempRenderer::Init();

    Renderer::Cubemap skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" };

    Player player{};

    unsigned int frames = 0;
    auto firstTime = nanoTime();
    auto lastSec = firstTime;

    while (!Window::shouldClose()) {
        auto nextTime = nanoTime();
        auto delta = nextTime - firstTime;
        firstTime = nextTime;

        float realDelta = delta / 1E9f;
        Window::pollUserEvents();
        Inputs::UpdateInputs();
        player.Step(realDelta);
        frames++;


        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        Renderer::CubemapRenderer::DrawCubemap(skybox, player.GetCamera(), player.GetPosition());
        TempRenderer::RenderCube({ 1, 0, 0 }, glm::vec3{ 2.f, .05f, .05f }, { 0.f, 0.f, 1.f }, player.GetCamera().getViewProjectionMatrix()); // +x blue
        TempRenderer::RenderCube({ 0, 1, 0 }, glm::vec3{ .05f, 2.f, .05f }, { 1.f, 0.f, 0.f }, player.GetCamera().getViewProjectionMatrix()); // +y red
        TempRenderer::RenderCube({ 0, 0, 1 }, glm::vec3{ .05f, .05f, 2.f }, { 0.f, 1.f, 0.f }, player.GetCamera().getViewProjectionMatrix()); // +z green
        TempRenderer::RenderCube({ 0, 0, 0 }, glm::vec3{ 1.f,  1.f,  1.f }, { .9f, .9f, .9f }, player.GetCamera().getViewProjectionMatrix()); // unit cube

        SceneManager::Step(realDelta);
        SceneManager::OnRender();
        SceneManager::OnImGuiRender();

        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            Window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }
        Window::sendFrame();
    }

    SceneManager::Shutdown();

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
