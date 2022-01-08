#include "Panels/ViewportPanel.h"

#include <imgui.h>
#include <glad/glad.h>

namespace GameGuy {



	ViewportPanel::ViewportPanel()
		: Panel("Viewport", false, true),
			mInstance(NULL),
			mResizeWidth(0),
			mResizeHeight(0),
			mNeedResize(false)
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

		ImVec2 size = ImGui::GetContentRegionAvail();
		ImVec2 newSize = { 0,0 };

		float offsetX, offsetY;
		offsetX = offsetY = 0.0f;

		if (size.y > size.x) {
			newSize.x = size.x;
			newSize.y = size.x * (144.0f / 160.0f);
		}
		else {
			newSize.x = size.y * (160.0f / 144.0f);
			newSize.y = size.y;
		}

		float scale = std::min(size.x / 160.0f, size.y / 144.0f);
		newSize.x = 160.0f * scale;
		newSize.y = 144.0f * scale;
		offsetX = (size.x - newSize.x) / 2;
		offsetY = (size.y - newSize.y) / 2;

		if (!mFBO) {
			uint32_t newWidth = (uint32_t)floor(size.x);
			uint32_t newHeight = (uint32_t)floor(size.y);
			mFBO = std::make_shared<FrameBuffer>(newWidth, newHeight);
		}
		else {
			uint32_t newWidth = (uint32_t)floor(size.x);
			uint32_t newHeight = (uint32_t)floor(size.y);
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
	}

}