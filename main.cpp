#include "src/abstraction/Window.h"
#include "src/abstraction/Renderer.h"
#include "src/abstraction/Camera.h"
#include "src/World/TerrainGeneration/MapGenerator.h"

#include "src/vendor/imgui/imgui.h"
#include "src/vendor/imgui/imgui_impl_glfw_gl3.h"

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


    //===========================================================//

    MapGenerator mapGen(100, 100, 27.6, 4, 0.3f, 3.18f, 235763);
    unsigned int id = mapGen.GenerateMap();
    std::shared_ptr<Renderer::Texture> texture = std::make_shared<Renderer::Texture>(id);





	Renderer::Camera m_Camera(-1.0f, 1.0f, -1.0f, 1.0f);
	Renderer::Renderer::Init();

    //------------------------------------//

    while (!Window::shouldClose()) {


        auto nextTime = nanoTime();
        auto delta = nextTime - firstTime;
        firstTime = nextTime;

        float realDelta = delta / 1E9f;
        Window::pollUserEvents();
        frames++;
                    
        ///////////////////////// BATCH
        

		Renderer::Renderer::Clear(0.f);
		Renderer::Renderer::BeginBatch(m_Camera);
        

        Renderer::Renderer::DrawQuad({ -1.f,-1.f, 0.0f }, { 2.f, 2.f }, texture);
                
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

        Window::sendFrame();
        temps += realDelta;


    }
	return 0;
}
