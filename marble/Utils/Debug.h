#pragma once

#if _DEBUG && defined(_MSC_VER)
#define MARBLE_DEBUGBREAK() __debugbreak();
#else
#define MARBLE_DEBUGBREAK()
#endif

/**
* A simple debug UI that displays render information
*/
namespace DebugWindow {

void onImGuiRender();
bool renderAABB();

}

