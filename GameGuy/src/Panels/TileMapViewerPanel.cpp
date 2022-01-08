#include "Panels/TileMapViewerPanel.h"

#include <imgui.h>
#include <glad/glad.h>

namespace GameGuy {
	TileMapViewerPanel::TileMapViewerPanel()
		:	Panel("Tile Map Viewer", false, true),
			mInstance(NULL),
			mResizeWidth(0),
			mResizeHeight(0),
			mNeedResize(false)
	{
	}

	TileMapViewerPanel::~TileMapViewerPanel()
	{
	}

	void TileMapViewerPanel::onSetup()
	{
		mBatchRenderer = std::make_shared<BatchRenderer>();
	}

	void TileMapViewerPanel::onRender()
	{
		if (mInstance && mFBO) {
			if (mNeedResize) {
				mFBO->resize(mResizeWidth, mResizeHeight);
				mFBO->bind();
				mNeedResize = false;
			}
			mFBO->bind();

			glViewport(0, 0, mFBO->width(), mFBO->height());
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			glm::mat4 projMatrix = glm::ortho(0.0f, (float)mFBO->width(), (float)mFBO->height(), 0.0f);
			mBatchRenderer->begin(projMatrix);

			float cellWidth = (float)mFBO->width() / 256.0f;
			float cellHeight = (float)mFBO->height() / 256.0f;
			glm::vec4 color;
			uint8_t pixels[8];

			for (uint8_t tileY = 0; tileY < 32; tileY++) {
				for (uint8_t tileX = 0; tileX < 32; tileX++) {
					uint8_t tileIndex = gbz80_ppu_tilemap_read_tile_index_by_coords(&mInstance->ppu, tileX, tileY);

					for (size_t pixelY = 0; pixelY < 8; pixelY++) {
						gbz80_ppu_read_tile_pixels_by_line(&mInstance->ppu, tileIndex, pixelY, pixels);

						for (size_t pixelX = 0; pixelX < 8; pixelX++) {
							uint8_t paletteColor = gbz80_ppu_get_bgp_color(&mInstance->ppu, pixels[pixelX]);

							switch (paletteColor) {
								case 0: color = { 1,1,1,1 }; break;
								case 1: color = { .82f,.82f,.82f,1 }; break;
								case 2: color = { .37f,.37f,.37f,1 }; break;
								case 3: color = { 0,0,0,1 }; break;
							}

							mBatchRenderer->drawQuad({ (tileX * 8 + pixelX) * cellWidth, (tileY * 8 + pixelY) * cellHeight }, { cellWidth, cellHeight }, color);
						}
					}

				}
			}

			mBatchRenderer->end();

			mFBO->unbind();
		}
	}

	void TileMapViewerPanel::onImGuiRender()
	{
		ImVec2 size = ImGui::GetContentRegionAvail();
		float offsetX, offsetY;

		float bestSize = std::min(size.x, size.y);
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
	}
}