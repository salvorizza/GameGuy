#pragma once

#include "Panel.h"

#include <filesystem>
#include <functional>

namespace GameGuy {

	class FileDialogPanel : public Panel {
	public:
		FileDialogPanel();
		~FileDialogPanel();


		inline void setOnFileSelectedCallback(const std::function<void(const char*)>& callback) { mOnFileSelected = callback; }
		inline void setCurrentPath(const std::filesystem::path& relativePath) { 
			std::filesystem::path currentPath = std::filesystem::absolute(relativePath);
			mHistory.clear();
			mHistory.push_back(currentPath);
			mSelectedPath = currentPath;
			mCurrentPath = mHistory.begin();
		}

		void setIconForExtension(const char* extension, const IconData& iconData, std::string_view type);
		void setFolderIcon(const IconData& iconData);

	protected:
		virtual void onImGuiRender() override;
		virtual void onOpen() override {}

	private:
		void selectFile(const std::filesystem::path& filePath);
		void selectNewPath(const std::filesystem::path& newPath);
		void refreshCurrentFolderContent();

		void renderPath(const std::filesystem::directory_entry& path, float thumbSize, float padding, float margin, float cellSize);

	private:
		struct ExtensionData {
			std::string type;
			IconData iconData;
		};

		std::filesystem::path mSelectedPath;
		std::vector<std::filesystem::path> mHistory;
		std::vector<std::filesystem::path>::iterator mCurrentPath;

		std::unordered_map<std::string, ExtensionData> mExtensionsIcons;
		IconData mFolderIcon;
		std::function<void(const char*)> mOnFileSelected;

	};

}