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
	Window::createWindow(813, 546, "test");

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
		//Renderer::Renderer::BeginBatch(m_Camera);

		//for (float y = -1.0f; y < 1.0f; y += 0.025f) {
		//	for (float x = -1.0f; x < 1.0f; x += 0.025f) {
		//		glm::vec4 color = { (x + 1.0f) / 2.0f * (sin(temps * 2) + 1),
		//							(x + y) / 2.0f * (cos(temps * 7) + 1),
		//							(y + 1.0f) / 2.0f * (sin(temps * 2) + 1),
		//							1.0f };
		//		Renderer::Renderer::DrawQuad({ x,y, 0.0f }, { 0.02f, 0.02f }, color);
		//	}
		//}

		//Renderer::Renderer::EndBatch();
		//Renderer::Renderer::Flush();

        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            Window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }

        Renderer::CubemapRenderer::DrawCubemap(skybox, player.GetCamera());
        TempRenderer::RenderCube({ 1, 0, 0 }, glm::vec3{ 1.f, .00f, .05f }, { 0.f, 0.f, 1.f }, player.GetCamera().getViewProjectionMatrix()); // +x blue
        TempRenderer::RenderPlane({ 0, 0.5, 0 }, glm::vec3{ 0.3f, 0.3F , 0.3F }, { (sin(temps)+1)*2, 0.f, 1.f}, 1, player.GetCamera().getViewProjectionMatrix(), lines);

        Window::sendFrame();
        temps += realDelta;
    }

	return 0;
}
