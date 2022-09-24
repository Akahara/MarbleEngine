#include "src/abstraction/Window.h"
#include "src/abstraction/Renderer.h"
#include "src/abstraction/Camera.h"
#include "src/abstraction/Cubemap.h"

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

    unsigned int frames = 0;
    Window::setVisible(true);
    Window::capFramerate();

    auto firstTime = nanoTime();
    auto lastSec = firstTime;
    auto temps = 0.F;

    //===========================================================//

	Renderer::Camera m_Camera(-1.0f, 1.0f, -1.0f, 1.0f);

	Renderer::Renderer::Init();
    Renderer::CubemapRenderer::Init();

    Renderer::Cubemap skybox{
      "res/skybox/skybox_front.bmp", "res/skybox/skybox_back.bmp",
      "res/skybox/skybox_left.bmp",  "res/skybox/skybox_right.bmp",
      "res/skybox/skybox_top.bmp",   "res/skybox/skybox_bottom.bmp" };


    while (!Window::shouldClose()) {
        auto nextTime = nanoTime();
        auto delta = nextTime - firstTime;
        firstTime = nextTime;

        float realDelta = delta / 1E9f;
        Window::pollUserEvents();
        frames++;


		Renderer::Renderer::Clear(0.f);
		Renderer::Renderer::BeginBatch(m_Camera);
        

		for (float y = -1.0f; y < 1.0f; y += 0.025f) {
			for (float x = -1.0f; x < 1.0f; x += 0.025f) {
                
				glm::vec4 color = { (x + 1.0f) / 2.0f * (sin(temps * 2) + 1),
									(x + y) / 2.0f * (cos(temps * 7) + 1),
									(y + 1.0f) / 2.0f * (sin(temps * 2) + 1),
									1.0f };
                
				Renderer::Renderer::DrawQuad({ x,y, 0.0f }, { 0.02f, 0.02f }, color);
			}
		}

		Renderer::Renderer::EndBatch();
		Renderer::Renderer::Flush();
        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            Window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }

        Renderer::CubemapRenderer::DrawCubemap(skybox, m_Camera);

        Window::sendFrame();
        temps += realDelta;
    }

	return 0;
}
