#pragma once

/*

#include "src/Events/Event.h"
#include "Window.h"
#include <GLFW/glfw3.h>
#include "Application.h"

#include <utility>

class Input {


public:

	inline static bool IsKeyPressed(int keycode) 
	
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	inline static bool IsMouseButtonPressed(int button) {

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	inline static std::pair<float, float> GetMousePosition() {

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { float(xpos), float(ypos) };


	}


	inline static float GetMouseX() {

		std::pair<float, float>res = GetMousePosition();
		return res.first;
	}

	inline static float GetMouseY() {
		std::pair<float, float>res = GetMousePosition();
		return res.second;
	}


private:

	// -- Singleton
	static Input* s_Instance;


};


*/