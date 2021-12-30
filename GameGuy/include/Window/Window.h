#pragma once

#include <cstdint>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace GameGuy {

	class Window {
	public:
		Window(const std::string& title,int32_t width,int32_t height);
		~Window();
		
		void show();
		void update();
		bool isClosed();

		GLFWwindow* getHandle() { return mWindowHandle; }
	private:
		GLFWwindow* mWindowHandle;
	};

}
