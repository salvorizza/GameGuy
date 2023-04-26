#pragma once

#include <memory>

#include "Window/Window.h"

#include <imgui.h>

#include <unordered_map>

namespace GameGuy {

	struct IconData {
		GLuint textureID;
		int32_t Width, Height;

		IconData()
			:	textureID(0),Width(0),Height(0)
		{}
	};

	class ImGuiManager {
	public:
		ImGuiManager(const std::shared_ptr<Window>& pWindow);
		~ImGuiManager();

		void startFrame();
		void endFrame();

		void setDarkThemeColors();

		ImFont* getSmallIconFont() { return mSmallIconFont; }
		ImFont* getLargeIconFont() { return mLargeIconFont; }

		IconData& LoadIconResource(const char* imagePath);
		IconData& LoadIconResource(const char* name, const std::vector<uint8_t>& data);
		IconData& GetIconResource(const char* name);
		bool ExistsIconResource(const char* name);

	private:
		std::shared_ptr<Window> mWindow;

		ImFont* mSmallIconFont;
		ImFont* mLargeIconFont;

		std::unordered_map<std::string, IconData> mIcons;
	};

}
