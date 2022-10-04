#include <thread>
#include <chrono>

#include "src/vendor/imgui/imgui.h"
#include "src/vendor/imgui/imgui_impl_glfw.h"
#include "src/vendor/imgui/imgui_impl_opengl3.h"

#include "src/abstraction/Window.h"
#include "src/abstraction/Inputs.h"
#include "src/Sandbox/Scene.h"
#include "src/World/Sky.h"
#include "src/Utils/Debug.h"
#include "src/Sandbox/Scenes/Test2D.h"
#include "src/Sandbox/Scenes/Test3D.h"
#include "src/Sandbox/Scenes/TestSky.h"
#include "src/Sandbox/Scenes/TestTerrain.h"
#include "src/Sandbox/Scenes/TestFB.h"

inline long long nanoTime()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

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
    Renderer::SkyRenderer::Init();
    Renderer::Init();
    SceneManager::Init();

    SceneManager::RegisterScene<Test2DScene>("Test");
    SceneManager::RegisterScene<Test3DScene>("Test3D");
    SceneManager::RegisterScene<TestTerrainScene>("Terrain");
    SceneManager::RegisterScene<TestSkyScene>("Sky");
    SceneManager::RegisterScene<TestFBScene>("Framebuffer");
    SceneManager::SwitchToScene(5);

    //===========================================================//

    unsigned int frames = 0;
    auto firstTime = nanoTime();
    auto lastSec = firstTime;

    while (!Window::shouldClose()) {

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
        DebugWindow::OnImGuiRender();

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
