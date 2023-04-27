#include "Panels/TileMapViewerPanel.h"

#include <imgui.h>
#include <glad/glad.h>

namespace GameGuy {
	TileMapViewerPanel::TileMapViewerPanel()
		:	Panel("Tile Map Viewer", false, true, false),
			mInstance(NULL),
			mResizeWidth(0),
			mResizeHeight(0),
			mNeedResize(false),
			mTilemapSelection(0),
			mRelativeAdressing(false)
	{
	}

	TileMapViewerPanel::~TileMapViewerPanel()
	{
	}

	void TileMapViewerPanel::onRender(const std::shared_ptr<BatchRenderer>& batchRenderer)
	{
		if (mOpen && mInstance && mFBO) {
			if (mNeedResize) {
				mFBO->resize(mResizeWidth, mResizeHeight);
				mFBO->bind();
				mNeedResize = false;
			}
			mFBO->bind();

			glViewport(0, 0, mFBO->width(), mFBO->height());
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			glm::mat4 projMatrix = glm::ortho(0.0f, (float)mFBO->width(), (float)mFBO->height(), 0.0f);

			float cellWidth = (float)mFBO->width() / 256.0f;
			float cellHeight = (float)mFBO->height() / 256.0f;

			batchRenderer->begin(projMatrix, 0.05, cellHeight / cellWidth);

			glm::vec4 color;
			uint8_t pixels[8];

			uint8_t tileYMax = 32;
			if (mTilemapSelection > 1) {
				tileYMax = 8;
			}


			for (uint8_t tileY = 0; tileY < tileYMax; tileY++) {
				for (uint8_t tileX = 0; tileX < 32; tileX++) {
					uint8_t tileIndex = tileY * 32 + tileX;
					if (mTilemapSelection <= 1) {
						tileIndex = gbz80_ppu_tilemap_read_tile_index_by_coords(&mInstance->ppu, tileX, tileY, (gbz80_ppu_tilemap_type_t)mTilemapSelection);
					}

					for (uint8_t pixelY = 0; pixelY < 8; pixelY++) {
						gbz80_ppu_read_tile_pixels_by_line(&mInstance->ppu, tileIndex, pixelY, pixels, mRelativeAdressing ? 0 : 1);

						for (uint8_t pixelX = 0; pixelX < 8; pixelX++) {
							uint8_t paletteColor = gbz80_ppu_get_bgp_color(&mInstance->ppu, pixels[pixelX]);

							switch (paletteColor) {
								case 0: color = { 1,1,1,1 }; break;
								case 1: color = { .66f,.66f,.66f,1 }; break;
								case 2: color = { .33f,.33f,.33f,1 }; break;
								case 3: color = { 0,0,0,1 }; break;
							}


							
							batchRenderer->drawQuad({ (tileX * 8 + pixelX) * cellWidth, (tileY * 8 + pixelY) * cellHeight }, { cellWidth, cellHeight }, color);
						}
					}

				}
			}

			batchRenderer->end();

			mFBO->unbind();
		}
	}

	void TileMapViewerPanel::onImGuiRender() {
		const char* items[] = { "Tilemap 0","Tilemap 1","Tile View" };
		ImGui::Combo(" ", &mTilemapSelection, items, IM_ARRAYSIZE(items));
		ImGui::Checkbox("Relative adressing", &mRelativeAdressing);

		ImVec2 size = ImGui::GetContentRegionAvail();
		float offsetX, offsetY;

		float bestSize = min(size.x, size.y);
		ImVec2 newSize = { bestSize,bestSize };
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

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 vMin;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		float cellWidth = (float)newSize.x / 32.0f;
		float cellHeight = (float)newSize.y / 32.0f;

		vMin.x = pos.x + offsetX;
		vMin.y = pos.y + cursorPos.y;

		for (int i = 0; i <= 32; i++) {
			drawList->AddLine(ImVec2(vMin.x + cellWidth * i, vMin.y), ImVec2(vMin.x + cellWidth * i, vMin.y + newSize.y), IM_COL32(0, 255, 0, 255));
			drawList->AddLine(ImVec2(vMin.x, vMin.y + cellHeight * i), ImVec2(vMin.x + newSize.x, vMin.y + cellHeight * i), IM_COL32(0, 255, 0, 255));
		}
	}
}