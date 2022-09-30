#pragma once

#define assert(x) if(!(x)) __debugbreak();

namespace DebugWindow {

void OnImGuiRender();

}