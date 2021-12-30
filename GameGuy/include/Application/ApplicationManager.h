#pragma once

#include <memory>

#include "Application.h"
#include "Window/ImGuiManager.h"
#include "Window/Window.h"

namespace GameGuy {

	class ApplicationManager {
	public:
		ApplicationManager();
		~ApplicationManager();

		void run(const std::shared_ptr<Application>& pApplication);
	private:
		std::shared_ptr<Application> mApplication;
		std::shared_ptr<ImGuiManager> mImGuiManager;
		std::shared_ptr<Window> mWindow;
	};

}
