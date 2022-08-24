#include "Panels/FileDialogPanel.h"
#include "Window/FontAwesome5.h"

#include <imgui.h>



namespace GameGuy {

	FileDialogPanel::FileDialogPanel()
		: Panel("FileDialog", false, false, false, true),
		mNavigatedPaths()
	{
		mRootPath = std::filesystem::current_path();
	}

	FileDialogPanel::~FileDialogPanel()
	{
	}

	void FileDialogPanel::setIconForExtension(const char* extension, const IconData& iconData)
	{
		mExtensionsIcons[extension] = iconData;
	}

	void FileDialogPanel::setFolderIcon(const IconData& iconData)
	{
		mFolderIcon = iconData;
	}

	void FileDialogPanel::onImGuiRender() {
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1, 0, 0, 1));

		if (ImGui::Button(ICON_FA_HOME)) {
			mNavigationIterator = mNavigatedPaths.begin();
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_CHEVRON_LEFT)) {
			if (mNavigationIterator > mNavigatedPaths.begin()) {
				mNavigationIterator--;
				refreshCurrentFolderContent();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_CHEVRON_RIGHT)) {
			if (mNavigationIterator < mNavigatedPaths.end() - 1) {
				mNavigationIterator++;
				refreshCurrentFolderContent();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_SYNC)) {
			refreshCurrentFolderContent();
		}
		ImGui::PopStyleColor(1);

		ImGui::SameLine();
		ImGui::Text(mNavigationIterator->string().c_str());
		ImGui::Separator();

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 windowPos = ImGui::GetWindowPos();


		float thumbSize = 96;
		float padding = 10;
		float margin = 10;

		float cellSize = thumbSize + padding;

		float availWidth = ImGui::GetContentRegionAvailWidth();
		int columnCount = std::max((int)(availWidth / (cellSize + margin)), 1);
		ImGui::Columns(columnCount, 0, false);
		ImVec2 size = { cellSize,cellSize * 1.75f };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

		for (const auto& directoryPath : mCurrentFolderContent) {
			std::string fileName = directoryPath.path().filename().string();
			std::string extension = directoryPath.path().extension().string();
			std::string type = "FILE";

			IconData iconData = directoryPath.is_directory() ? mFolderIcon : mExtensionsIcons.at(".*");
			if (mExtensionsIcons.find(extension) != mExtensionsIcons.end()){
				iconData = mExtensionsIcons.at(extension);
			}

			ImVec2 textSize = ImGui::CalcTextSize(fileName.c_str());
			ImVec2 screenPos = ImGui::GetCursorScreenPos();

			if (directoryPath.is_directory()) {
				drawList->AddImageRounded(
					(void*)(intptr_t)iconData.textureID,
					{ screenPos.x + padding / 2, screenPos.y + padding / 2 + (size.y - thumbSize) / 2 - textSize.y},
					{ screenPos.x + (padding / 2) + thumbSize,screenPos.y + (padding / 2) + thumbSize + (size.y - thumbSize) / 2 - textSize.y },
					{ 0,0 },
					{ 1,1 },
					IM_COL32(255, 255, 255, 255),
					3
				);
				drawList->AddText(
					{ screenPos.x + (padding / 2) + (thumbSize - textSize.x) / 2,screenPos.y + (padding / 2) + thumbSize + (size.y - thumbSize) / 2 },
					IM_COL32(255, 255, 255, 255),
					fileName.c_str()
				);

				ImGui::InvisibleButton(fileName.c_str(), size);
				if (ImGui::IsItemHovered()) {
					drawList->AddRect(
						{ screenPos.x,screenPos.y },
						{ screenPos.x + size.x,screenPos.y + size.y },
						IM_COL32(97, 84, 63, 255),
						3,
						0,
						2
					);
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					selectNewPath(directoryPath.path());
					break;
				}
				ImGui::NextColumn();
			}
			else {
				/*Shadow Box*/
				drawList->AddRectFilled(
					{ screenPos.x,screenPos.y },
					{ screenPos.x + size.x + 1,screenPos.y + size.y + 1 },
					IM_COL32(0, 0, 0, 255),
					1
				);
				/*BG Box*/
				drawList->AddRectFilled(
					{ screenPos.x,screenPos.y },
					{ screenPos.x + size.x,screenPos.y + size.y },
					IM_COL32(26, 26, 26, 255),
					3
				);
				/*Label Box*/
				drawList->AddRectFilled(
					{ screenPos.x,screenPos.y + thumbSize + padding * 2 },
					{ screenPos.x + size.x,screenPos.y + size.y },
					IM_COL32(47, 47, 47, 255),
					3
				);
				/*Icon*/
				drawList->AddImageRounded(
					(void*)(intptr_t)iconData.textureID,
					{ screenPos.x + padding / 2,screenPos.y + padding },
					{ screenPos.x + (padding / 2) + thumbSize,screenPos.y + padding + thumbSize },
					{ 0,0 },
					{ 1,1 },
					IM_COL32(255, 255, 255, 255),
					3
				);
				/*Label*/
				drawList->AddText(
					{ screenPos.x + padding / 2,screenPos.y + thumbSize + padding * 2.5f },
					IM_COL32(255, 255, 255, 255),
					fileName.c_str()
				);
				/*Type*/
				ImVec2 typeTextSize = ImGui::CalcTextSize(type.c_str());
				drawList->AddText(
					{ screenPos.x + size.x - typeTextSize.x - padding /2,screenPos.y + size.y - typeTextSize.y - padding /2 },
					IM_COL32(255, 255, 255, 255),
					type.c_str()
				);


				ImGui::InvisibleButton(fileName.c_str(), size);
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
					drawList->AddRectFilled(
						{ screenPos.x,screenPos.y },
						{ screenPos.x + size.x,screenPos.y + size.y },
						IM_COL32(255, 255, 255, 75),
						3,
						0
					);
				}

				if (ImGui::IsItemHovered()) {
					drawList->AddRect(
						{ screenPos.x,screenPos.y },
						{ screenPos.x + size.x,screenPos.y + size.y },
						IM_COL32(97, 84, 63, 255),
						3,
						0,
						2
					);
				}

				
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					selectFile(directoryPath.path());
					break;
				}
				ImGui::NextColumn();
			}
		}

		ImGui::PopStyleColor(3);
		ImGui::Columns(1);
	}

	void FileDialogPanel::selectFile(const std::filesystem::path& filePath) {
		mOnFileSelected(filePath.string().c_str());
		mOpen = false;
	}

	void FileDialogPanel::selectNewPath(const std::filesystem::path& newPath)
	{
		mNavigatedPaths.push_back(newPath);
		mNavigationIterator = mNavigatedPaths.end() - 1;
		refreshCurrentFolderContent();
	}

	void FileDialogPanel::refreshCurrentFolderContent()
	{
		mCurrentFolderContent.clear();
		for (const auto& directoryPath : std::filesystem::directory_iterator(*mNavigationIterator)) {
			mCurrentFolderContent.emplace_back(directoryPath);
		}

		std::sort(mCurrentFolderContent.begin(), mCurrentFolderContent.end(), [](auto a, auto b) {
			return a.path().string() < b.path().string();
			});

		std::sort(mCurrentFolderContent.begin(), mCurrentFolderContent.end(), [](auto a, auto b) {
			return int(a.is_directory()) > int(b.is_directory());
			});
	}

}