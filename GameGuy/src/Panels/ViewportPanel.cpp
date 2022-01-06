#include "Panels/ViewportPanel.h"

#include <glad/glad.h>
#include <imgui.h>

namespace GameGuy {



	ViewportPanel::ViewportPanel()
		: Panel("Viewport", false, true)
	{}

	ViewportPanel::~ViewportPanel()
	{}

	void ViewportPanel::onRender()
	{
		if (mFBO) {
			mFBO->bind();
			glClearColor(0, 1, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
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
			offsetY = (size.y - newSize.y) / 2;
		}
		else {
			newSize.x = size.y * (160.0f / 144.0f);
			newSize.y = size.y;
			offsetX = (size.x - newSize.x) / 2;
		}


		if (!mFBO) {
			mFBO = std::make_shared<FrameBuffer>(newSize.x, newSize.y);
		}

		
		uint64_t textureID = mFBO->getColorAttachment();
		ImVec2 cursorPos = ImGui::GetCursorPos();
		cursorPos.x += offsetX;
		cursorPos.y += offsetY;
		ImGui::SetCursorPos(cursorPos);
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ newSize.x, newSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
	}

}