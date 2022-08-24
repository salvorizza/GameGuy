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
		inline void setRoot(const char* relativeRoot) { mRootPath = std::filesystem::relative(relativeRoot); }

		void setIconForExtension(const char* extension, const IconData& iconData);
		void setFolderIcon(const IconData& iconData);

	protected:
		virtual void onImGuiRender() override;
		virtual void onOpen() override {
			selectNewPath(mRootPath);
		}

	private:
		void selectFile(const std::filesystem::path& filePath);
		void selectNewPath(const std::filesystem::path& newPath);
		void refreshCurrentFolderContent();

	private:
		std::filesystem::path mRootPath;
		std::vector<std::filesystem::path> mNavigatedPaths;
		std::vector<std::filesystem::path>::iterator mNavigationIterator;
		std::vector<std::filesystem::directory_entry> mCurrentFolderContent;

		std::unordered_map<std::string, IconData> mExtensionsIcons;
		IconData mFolderIcon;
		std::function<void(const char*)> mOnFileSelected;

	};

}