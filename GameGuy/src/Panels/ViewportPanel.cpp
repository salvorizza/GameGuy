#include "Panels/ViewportPanel.h"

#include <imgui.h>
#include <glad/glad.h>

namespace GameGuy {



	ViewportPanel::ViewportPanel()
		: Panel("Viewport", false, true),
			mInstance(NULL),
			mResizeWidth(160),
			mResizeHeight(144),
			mNeedResize(true)
	{}

	ViewportPanel::~ViewportPanel()
	{}

	void ViewportPanel::startFrame()
	{
		if (mFBO) {
			if (mNeedResize) {
				mFBO->resize(mResizeWidth, mResizeHeight);
				mNeedResize = false;
			}
			mFBO->bind();

			glViewport(0, 0, mFBO->width(), mFBO->height());

		}
	}

	void ViewportPanel::endFrame()
	{
		if (mFBO) {
			mFBO->unbind();
		}
	}

	void ViewportPanel::onImGuiRender()
	{
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetContentRegionAvail();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 newSize = { 0,0 };


		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		vMin.x += pos.x;
		vMin.y += pos.y;

		float offsetX, offsetY;
		offsetX = offsetY = 0.0f;

		float scale = floor(std::min(size.x / 160.0f, size.y / 144.0f));
		newSize.x = 160.0f * scale;
		newSize.y = 144.0f * scale;
		offsetX = (size.x - newSize.x) / 2;
		offsetY = (size.y - newSize.y) / 2;

		if (!mFBO) {
			uint32_t newWidth = (uint32_t)floor(newSize.x);
			uint32_t newHeight = (uint32_t)floor(newSize.y);
			mFBO = std::make_shared<FrameBuffer>(newWidth, newHeight);
		}
		else {
			uint32_t newWidth = (uint32_t)floor(newSize.x);
			uint32_t newHeight = (uint32_t)floor(newSize.y);
			uint32_t fboWidth, fboHeight;

			mFBO->getSize(fboWidth, fboHeight);
			if (newWidth != fboWidth || newHeight != fboHeight) {
				mResizeWidth = newWidth;
				mResizeHeight = newHeight;
				mNeedResize = true;
			}
		}
		
		uint64_t textureID = mFBO->getColorAttachment();
		ImVec2 cursorPos = ImGui::GetCursorPos();
		cursorPos.x += offsetX;
		cursorPos.y += offsetY;
		ImGui::SetCursorPos(cursorPos);
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ newSize.x, newSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		float cellWidth = (float)newSize.x / 20.0f;//20
		float cellHeight = (float)newSize.y / 18.0f;//18

		vMin.x += offsetX;
		vMin.y += offsetY;

		/*
		for (int i = 0; i <= 20; i++) {
			drawList->AddLine(ImVec2(vMin.x + cellWidth * i, vMin.y), ImVec2(vMin.x + cellWidth * i, vMin.y + newSize.y), IM_COL32(0, 255, 0, 255));

			if(i <= 18)
				drawList->AddLine(ImVec2(vMin.x, vMin.y + cellHeight * i), ImVec2(vMin.x + newSize.x, vMin.y + cellHeight * i), IM_COL32(0, 255, 0, 255));
		}
		*/

	}

}