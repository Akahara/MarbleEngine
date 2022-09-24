#include "src/abstraction/Window.h"
#include "src/abstraction/Renderer.h"
#include "src/abstraction/Camera.h"
#include "src/World/TerrainGeneration/MapGenerator.h"

#include "src/vendor/imgui/imgui.h"
#include "src/vendor/imgui/imgui_impl_glfw.h"
#include "src/vendor/imgui/imgui_impl_opengl3.h"

#include <thread>
#include <chrono>

inline long long nanoTime()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}


int main()
{
	Window::createWindow(813, 546, "test");

    unsigned int  frames = 0;
    Window::setVisible(true);
    Window::capFramerate();

    auto firstTime = nanoTime();

    auto lastSec = firstTime;

    auto temps = 0.F;

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(Window::GetWindowHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    //===========================================================//

    MapGenerator mapGen(100, 100, 27.6, 4, 0.3f, 3.18f, 235763);
    unsigned int id = mapGen.GenerateMap();
    std::shared_ptr<Renderer::Texture> texture = std::make_shared<Renderer::Texture>(id);
    std::shared_ptr<Renderer::Texture> newTexture;

    int w=100, h=100;
    float scale=27.6;
    int o=4;
    float p=0.3, l=3.18;
    int seed= 235763;


	Renderer::Camera m_Camera(-1.0f, 1.0f, -1.0f, 1.0f);


	Renderer::Renderer::Init();
    bool a = false;
    //------------------------------------//

    while (!Window::shouldClose()) {


        auto nextTime = nanoTime();
        auto delta = nextTime - firstTime;
        firstTime = nextTime;

        float realDelta = delta / 1E9f;
        Window::pollUserEvents();
        frames++;

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        /*
        ImGui::SliderInt("Width", &w, 0, 300);
        ImGui::SliderInt("Height",&h , 0, 300); 
        ImGui::SliderFloat("Scale",&scale, 0, 50);
        ImGui::SliderInt("Number of octaves",&o, 0, 5);
        ImGui::SliderFloat("persistence",&p, 0, 1);
        ImGui::SliderFloat("lacunarity",&l, 0, 10);
        ImGui::SliderInt("seed",&seed, 0, 30000);
        */
                    

        if (ImGui::SliderInt("Width", &w, 0, 300) || ImGui::SliderInt("Height", &h, 0, 300) ||
            ImGui::SliderFloat("Scale", &scale, 0, 50) || ImGui::SliderInt("Number of octaves", &o, 0, 5) ||
            ImGui::SliderFloat("persistence", &p, 0, 1) || ImGui::SliderFloat("lacunarity", &l, 0, 10) ||
            ImGui::SliderInt("seed", &seed, 0, 30000) ) {

            unsigned int nid;

            float* noiseMap = Noise::GenerateNoiseMap(w,h,scale,o,p,l,seed);

            // Texture stuff

            (glGenTextures(1, &nid));
            (glBindTexture(GL_TEXTURE_2D, nid));

            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            (glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            if (noiseMap)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, noiseMap);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            std::cout << "End of generation" << std::endl;
            delete[] noiseMap;

            newTexture = std::make_shared<Renderer::Texture>(nid);
            a = true;


        }
        ///////////////////////// BATCH
        

		Renderer::Renderer::Clear(0.f);
		Renderer::Renderer::BeginBatch(m_Camera);
        

        Renderer::Renderer::DrawQuad({ -1.f,-1.f, 0.0f }, { 2.f, 2.f }, texture);
        if (a)
            Renderer::Renderer::DrawQuad({ -1.f,-1.f, 0.0f }, { 2.f, 2.f }, newTexture);
                
		Renderer::Renderer::EndBatch();
		Renderer::Renderer::Flush();
        
        /////////////////////////////////

        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            Window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        Window::sendFrame();
        temps += realDelta;


    }


    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
	return 0;
}
