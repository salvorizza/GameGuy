#include "Application/ApplicationManager.h"

namespace GameGuy {
	ApplicationManager::ApplicationManager()
	{
	}

	ApplicationManager::~ApplicationManager()
	{
	}

	void ApplicationManager::run(const std::shared_ptr<Application>& pApplication)
	{
		mApplication = pApplication;
		mWindow = std::make_shared<Window>(mApplication->getName(), 1280, 720);
		mImGuiManager = std::make_shared<ImGuiManager>(mWindow);

		mWindow->show();
		while (!mWindow->isClosed()) {
			mApplication->onUpdate();
			mApplication->onRender();

			mImGuiManager->startFrame();
			mApplication->onImGuiRender();
			mImGuiManager->endFrame();

			mWindow->update();
		}
	}

}