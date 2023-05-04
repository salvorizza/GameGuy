#include "Panels/FileDialogPanel.h"
#include "Window/FontAwesome5.h"

#include <imgui.h>

#include <curl/curl.h>

namespace GameGuy {


	FileDialogPanel::FileDialogPanel()
		: Panel("FileDialog", false, ImVec4(0.14f, 0.14f, 0.14f, 1.00f), false, true, false)
	{;
		std::filesystem::path currentPath = std::filesystem::current_path();
		mCurrentPath = mHistory.end();
		selectNewPath(currentPath);
	}

	FileDialogPanel::~FileDialogPanel()
	{
	}

	void FileDialogPanel::setIconForExtension(const char* extension, const IconData& iconData, std::string_view type)
	{
		ExtensionData extData;
		extData.iconData = iconData;
		extData.type = type;

		mExtensionsIcons[extension] = extData;
	}

	void FileDialogPanel::setFolderIcon(const IconData& iconData)
	{
		mFolderIcon = iconData;
	}

	void FileDialogPanel::onImGuiRender() {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		{
			bool condition = mCurrentPath > mHistory.begin();
			if (!condition) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.14f, 0.14f, 1));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.14f, 1));
			}

			if (ImGui::Button(ICON_FA_ARROW_CIRCLE_LEFT) && condition) {
				mCurrentPath--;
			}
			
			if (!condition) {
				ImGui::PopStyleColor(3);
			}
		}
		ImGui::SameLine();
		{
			bool condition = mCurrentPath < (mHistory.end() - 1);
			if (!condition) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.14f, 0.14f, 1));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.14f, 1));
			}

			if (ImGui::Button(ICON_FA_ARROW_CIRCLE_RIGHT) && condition) {
				mCurrentPath++;
			}

			if (!condition) {
				ImGui::PopStyleColor(3);
			}
		}

		std::filesystem::path buttonPath = "";
		for (auto& directoryPath : *mCurrentPath) {
			bool breakFor = false;

			buttonPath /= directoryPath;

			ImGui::PushID(buttonPath.string().c_str());
			ImGui::SameLine();
			if (ImGui::Button(directoryPath.string().c_str())) {
				selectNewPath(buttonPath);
				breakFor = true;
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_CHEVRON_RIGHT)) ImGui::OpenPopup("FolderPopup");

			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.21f, 0.21f, 0.21f, 1));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.00f, 0.43f, 0.87f, 1));
			if (ImGui::BeginPopup("FolderPopup")) {
				for (auto& folderPath : std::filesystem::directory_iterator{ buttonPath }) {
					if (folderPath.is_directory()) {
						std::string fileName = folderPath.path().stem().string();
						if (ImGui::MenuItem((ICON_FA_FOLDER + std::string("  ") + fileName).c_str())) {
							selectNewPath(folderPath);
							breakFor = true;
							break;
						}
					}
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleColor(2);

			ImGui::PopID();

			if (breakFor) {
				break;
			}
		}
		ImGui::PopStyleVar();

		ImGui::Separator();


		float thumbSize = 72;
		float padding = 10;
		float margin = 10;
		float cellSize = thumbSize + padding;

		float availWidth = ImGui::GetContentRegionAvailWidth();
		int columnCount = max((int)(availWidth / (cellSize + margin)), 1);
		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryPath : std::filesystem::directory_iterator{ *mCurrentPath }) {
			if (directoryPath.is_directory()) {
				renderPath(directoryPath, thumbSize, padding, margin, cellSize);
				ImGui::NextColumn();
			}
		}

		for (auto& directoryPath : std::filesystem::directory_iterator{ *mCurrentPath }) {
			if (!directoryPath.is_directory()) {
				renderPath(directoryPath, thumbSize, padding, margin, cellSize);
				ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);
	}

	void FileDialogPanel::selectFile(const std::filesystem::path& filePath) {
		mOnFileSelected(filePath.string().c_str());
		//mOpen = false;
	}

	void FileDialogPanel::selectNewPath(const std::filesystem::path& newPath)
	{
		if (mCurrentPath != mHistory.end() && mCurrentPath < mHistory.end() - 1) {
			mHistory.erase(mCurrentPath + 1, mHistory.end());
		}
		mHistory.push_back(newPath);
		mCurrentPath = mHistory.end() - 1;

		for (auto& title : mIconsCache) {
			if (mManager->ExistsIconResource(title.c_str())) {
				mManager->ReleaseIconResource(title.c_str());
			}
		}
		mIconsCache.clear();
	}

	HTTPResponse FileDialogPanel::loadCoverFromTitle(const std::string& title)
	{
		CURL* curl = HTTPInit();

		static std::string baseURL = "https://raw.githubusercontent.com/libretro/libretro-thumbnails/master/Nintendo%20-%20Game%20Boy/Named_Boxarts/";
		std::string httpURL = baseURL + HTTPURLEncode(curl, title.c_str()) + ".png";
		DataBuffer dataBuffer;

		HTTPResponse response = HTTPGet(curl, httpURL.c_str());
		HTTPClose(curl);

		std::lock_guard lc(mIconsCacheMutex);
		mIconsCache.insert(title);

		return response;
	}

	void FileDialogPanel::renderPath(const std::filesystem::directory_entry& path,float thumbSize,float padding, float margin,float cellSize)
	{

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 windowPos = ImGui::GetWindowPos();

		ImVec2 size = { cellSize,cellSize * 1.75f };

		std::string stem = path.path().stem().string();
		std::string fileName = stem;
		std::string extension = path.path().extension().string();
		std::string type = "FILE";
		bool isDirectory = path.is_directory();
		bool isSelected = mSelectedPath == path;


		IconData iconData = path.is_directory() ? mFolderIcon : mExtensionsIcons.at(".*").iconData;
		auto it = mExtensionsIcons.find(extension);
		if (it != mExtensionsIcons.end()) {
			iconData = it->second.iconData;
			type = it->second.type;
		}

		ImVec2 screenPos = ImGui::GetCursorScreenPos();

		ImGui::InvisibleButton(fileName.c_str(), size);

		ImU32 labelBoxColor = isSelected ? IM_COL32(0, 112, 224, 255) : (ImGui::IsItemHovered() ? IM_COL32(87, 87, 87, 255) : IM_COL32(56, 56, 56, 255));

		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			mSelectedPath = path;
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (!isDirectory) {
				selectFile(path);
			}
			else {
				selectNewPath(path);
			}
			return;
		}


		/*BG Box*/
		ImU32 bgBoxColor = isDirectory && !isSelected ? (!ImGui::IsItemHovered() ? IM_COL32(35, 35, 35, 255) : IM_COL32(56, 56, 56, 255)) : IM_COL32(23, 23, 23, 255);
		drawList->AddRectFilled(
			{ screenPos.x,screenPos.y },
			{ screenPos.x + size.x,screenPos.y + size.y },
			bgBoxColor,
			3
		);

		if (!isDirectory || isSelected) {
			/*Label Box*/
			drawList->AddRectFilled(
				{ screenPos.x,screenPos.y + thumbSize + padding * 2 },
				{ screenPos.x + size.x,screenPos.y + size.y - 10 },
				labelBoxColor,
				0
			);

			drawList->AddRectFilled(
				{ screenPos.x,screenPos.y + thumbSize + padding * 2 },
				{ screenPos.x + size.x,screenPos.y + size.y },
				labelBoxColor,
				3
			);

			if (ImGui::IsItemHovered() || isSelected) {
				drawList->AddRect(
					{ screenPos.x,screenPos.y },
					{ screenPos.x + size.x,screenPos.y + size.y },
					labelBoxColor,
					3,
					0,
					1
				);
			}
		}
		

		ImTextureID textureID = (void*)(intptr_t)iconData.textureID;
		bool isLoading = false;
		if (!path.is_directory()) {
			isLoading = true;

			IconData coverData;

			std::lock_guard lc(mIconsCacheMutex);
			auto it = std::find(mIconsCache.begin(), mIconsCache.end(), stem);
			auto itFuture = mFutures.find(stem);

			if (it != mIconsCache.end()) {
				isLoading = false;
				if (itFuture != mFutures.end()) {
					HTTPResponse response = itFuture->second.get();
					if (response.Status == 200) {
						coverData = mManager->LoadIconResource(stem.c_str(), response.Body.Data, response.Body.Size);
						DeleteBuffer(response.Body);
					}
					mFutures.erase(itFuture);
				} else {
					coverData = mManager->GetIconResource(stem.c_str());
				}
			} else {
				if (itFuture == mFutures.end()) {
					mFutures[stem] = std::async(std::launch::async, &FileDialogPanel::loadCoverFromTitle, this, stem);
				}
			}
			
			if (coverData.textureID != 0) {
				textureID = (void*)(intptr_t)coverData.textureID;
			}
		}

		if (isLoading) {
			drawList->PathClear();
			float angle = 2 * 3.1415f * sinf(ImGui::GetTime());
			drawList->PathArcTo({ screenPos.x + padding / 2 + thumbSize / 2,screenPos.y + padding + thumbSize / 2 }, thumbSize / 3, angle, angle + 3.1415f);
			drawList->PathStroke(IM_COL32(0, 112, 224, 255), 0, 4);
		} else {
			/*Icon*/
			drawList->AddImageRounded(
				textureID,
				{ screenPos.x + padding / 2,screenPos.y + padding },
				{ screenPos.x + (padding / 2) + thumbSize,screenPos.y + padding + thumbSize },
				{ 0,0 },
				{ 1,1 },
				IM_COL32(255, 255, 255, 255),
				3
			);
		}

		ImVec4 labelClipRect = ImVec4(
			screenPos.x + padding / 2, screenPos.y + thumbSize + padding * 2.5f,
			screenPos.x + size.x - padding / 2, screenPos.y + size.y - (ImGui::GetFontSize() - 4)
		);

		/*Label*/
		drawList->AddText(
			ImGui::GetFont(),
			ImGui::GetFontSize() - 2,
			{ screenPos.x + padding / 2,screenPos.y + thumbSize + padding * 2.5f },
			IM_COL32(255, 255, 255, 255),
			path.path().filename().string().c_str(),
			(const char*)0,
			size.x,
			&labelClipRect
		);
	}

}