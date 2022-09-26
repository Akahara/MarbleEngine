#include "src/abstraction/Window.h"
#include "src/abstraction/Renderer.h"
#include "src/abstraction/Camera.h"
#include "src/abstraction/Cubemap.h"
#include "src/abstraction/Inputs.h"

#include "src/abstraction/TempRenderer.h"

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

    //===========================================================//

	Renderer::Camera m_Camera(-1.0f, 1.0f, -1.0f, 1.0f);

	Renderer::Renderer::Init();
    Renderer::CubemapRenderer::Init();
    TempRenderer::Init();

    Renderer::Cubemap skybox{
      "res/skybox_dbg/skybox_front.bmp", "res/skybox_dbg/skybox_back.bmp",
      "res/skybox_dbg/skybox_left.bmp",  "res/skybox_dbg/skybox_right.bmp",
      "res/skybox_dbg/skybox_top.bmp",   "res/skybox_dbg/skybox_bottom.bmp" };

    Player player{};

    while (!Window::shouldClose()) {
        auto nextTime = nanoTime();
        auto delta = nextTime - firstTime;
        firstTime = nextTime;

        float realDelta = delta / 1E9f;
        Window::pollUserEvents();
        Inputs::UpdateInputs();
        player.Step(realDelta);
        frames++;


		Renderer::Renderer::Clear(0.f);


        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            Window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }

        TempRenderer::RenderGrid({ -1, -1, 0 }, 2.f, 1000, { 1.f, 1.f, 1.f }, player.GetCamera().getViewProjectionMatrix(), false);

        Window::sendFrame();
        temps += realDelta;
    }

	return 0;
}
