#include <thread>
#include <chrono>

#include "marble/vendor/imgui/imgui.h"
#include "marble/vendor/imgui/imgui_impl_glfw.h"
#include "marble/vendor/imgui/imgui_impl_opengl3.h"

#include "marble/abstraction/Window.h"
#include "marble/abstraction/Inputs.h"

#include "marble/Sandbox/Scene.h"
#include "marble/Utils/Debug.h"

#include "marble/Sandbox/Tests.h"

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
    Inputs::observeInputs();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(Window::getWindowHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Renderer::SkyRenderer::init();
    Renderer::init();
    SceneManager::init();

    
    SceneManager::registerScene<TestTerrainScene>("Terrain");
    SceneManager::registerScene<TestSkyScene>("Sky");
    SceneManager::registerScene<TestFBScene>("Framebuffer");
    SceneManager::registerScene<TestShadersScene>("Shaders");
    SceneManager::registerScene<TestShadowsScene>("Shadows");
    SceneManager::registerScene<TestCamerasScene>("Cameras");
    SceneManager::registerScene<TestComputeShader>("Compute Shader");
    SceneManager::registerScene<TestInstancedScene>("Instanced");
    SceneManager::registerScene<TestBloomScene>("Bloom");
    SceneManager::registerScene<TestAtlas>("Atlas");
    SceneManager::registerScene<TestWater>("Water");
    SceneManager::registerScene<POC1Scene>("POC 1");
    SceneManager::registerScene<POC2Scene>("POC 2");
    SceneManager::registerScene<POC3Scene>("POC 3");
    SceneManager::registerScene<POC4Scene>("POC 4");
    

    SceneManager::switchToScene(5);

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
        Inputs::updateInputs();
        frames++;

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        
        SceneManager::step(realDelta);
        SceneManager::onRender();
        SceneManager::onImGuiRender();
        DebugWindow::onImGuiRender();

        Renderer::Shader::unbind(); // unbind shaders before ImGui's new frame, so it won't try to restore a shader that has been deleted

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

    SceneManager::shutdown();

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

	return 0;
}
