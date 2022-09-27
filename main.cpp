#include "src/abstraction/Window.h"
#include "src/abstraction/Renderer.h"
#include "src/abstraction/Camera.h"
#include "src/abstraction/Cubemap.h"
#include "src/abstraction/Inputs.h"

#include "src/abstraction/TempRenderer.h"
#include "src/world/TerrainGeneration/MapGenerator.h"


#include "src/vendor/imgui/imgui.h"
#include "src/vendor/imgui/imgui_impl_glfw.h"
#include "src/vendor/imgui/imgui_impl_opengl3.h"


#include "src/world/Player.h"

#include <thread>
#include <chrono>

inline long long nanoTime()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}


int main()
{
	Window::createWindow(1000, 1000, "test");

    unsigned int frames = 0;
    Window::setVisible(true);
    Window::setPosition(400, 100);
    Window::capFramerate();
    Inputs::ObserveInputs();

    auto firstTime = nanoTime();
    auto lastSec = firstTime;
    auto temps = 0.F;

    bool lines = true;


    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(Window::GetWindowHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    //===========================================================//

    int w=200, h=200;
    float scale=27.6;
    int o=4;
    float p=0.3, l=3.18;
    int seed= 5;

    MapGenerator mapGen(w, h, scale, o, p, l, seed);
    unsigned int id = mapGen.GenerateTextureMap();
    float* noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);


	Renderer::Camera m_Camera(-1.0f, 1.0f, -1.0f, 1.0f);


    Renderer::Renderer::Init();
    Renderer::CubemapRenderer::Init();
    TempRenderer::Init();

    Player player{};

    while (!Window::shouldClose()) {

        Renderer::Renderer::Clear(0.f);
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
        
        if (ImGui::SliderInt("Width", &w, 0, 300) + ImGui::SliderInt("Height", &h, 0, 300) +
            ImGui::SliderFloat("Scale", &scale, 0, 50) + ImGui::SliderInt("Number of octaves", &o, 0, 5) +
            ImGui::SliderFloat("persistence", &p, 0, 1) + ImGui::SliderFloat("lacunarity", &l, 0, 10) +
            ImGui::SliderInt("seed", &seed, 0, 5)) {

            delete[] noiseMap;
            unsigned int nid;

            noiseMap = Noise::GenerateNoiseMap(w, h, scale, o, p, l, seed);

            // Texture stuff

            (glGenTextures(1, &nid));
            (glBindTexture(GL_TEXTURE_2D, nid));

            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            if (noiseMap)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_FLOAT, noiseMap);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            

            id = nid;


        }
                    




        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            Window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }

        if (noiseMap) {

            TempRenderer::RenderGrid({ -1, -1, 0 }, 30.f, 128, { 1.f, 1.f, 1.f }, player.GetCamera().getViewProjectionMatrix(), id, noiseMap, false);
            
        }

        temps += realDelta;

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        Window::sendFrame();


    }

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

	return 0;
}
