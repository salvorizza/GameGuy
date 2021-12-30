#include "Window/Window.h"

namespace GameGuy {
	Window::Window(const std::string& title, int32_t width, int32_t height)
		:	mWindowHandle(NULL)
	{
		if (!glfwInit()) {
			return;
		}

		GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		glfwSwapInterval(1);

		mWindowHandle = window;
	}

	Window::~Window()
	{
		glfwDestroyWindow(mWindowHandle);
		glfwTerminate();
	}

	void Window::show()
	{
		glfwShowWindow(mWindowHandle);
	}

	void Window::update()
	{
		glfwSwapBuffers(mWindowHandle);
		glfwPollEvents();
	}

	bool Window::isClosed()
	{
		return glfwWindowShouldClose(mWindowHandle);
	}
}