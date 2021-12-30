#pragma once

#include <memory>

#include "Window/Window.h"

namespace GameGuy {

	class ImGuiManager {
	public:
		ImGuiManager(const std::shared_ptr<Window>& pWindow);
		~ImGuiManager();

		void startFrame();
		void endFrame();

		void setDarkThemeColors();
	private:
		std::shared_ptr<Window> mWindow;
	};

}
