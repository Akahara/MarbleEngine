#include "src/DisplayCentral/window.h"

#include "src/abstraction/Renderer.h"
#include "src/abstraction/Camera.h"


#include <thread>
#include<chrono>

inline long long nanoTime()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}


int main()
{
	window::createWindow(813, 546, "test");

    unsigned int  frames = 0;
    window::setVisible(true);
    window::capFramerate();

    auto firstTime = nanoTime();

    auto lastSec = firstTime;

    auto temps = 0.F;


    //===========================================================//

	Renderer::Camera m_Camera(-1.0f, 1.0f, -1.0f, 1.0f);

	Renderer::Renderer::Init();


    while (!window::shouldClose()) {
        auto nextTime = nanoTime();
        auto delta = nextTime - firstTime;
        firstTime = nextTime;

        float realDelta = delta / 1E9f;
        window::pollUserEvents();
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

        std::cout << temps << std::endl;

        
		Renderer::Renderer::EndBatch();
		Renderer::Renderer::Flush();
        if (lastSec + 1E9 < nextTime) {
            char title[50];
            sprintf_s(title, 50, "Some game, %dfps", frames);
            window::renameWindow(title);
            lastSec += (long long)1E9;
            frames = 0;
        }
        window::sendFrame();
        temps += realDelta;
    }
	return 0;
}
