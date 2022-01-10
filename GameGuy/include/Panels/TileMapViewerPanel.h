#pragma once

#include <gbz80.h>

#include "Panel.h"

#include "Graphics/BatchRenderer.h"
#include "Graphics/FrameBuffer.h"

namespace GameGuy {

	class TileMapViewerPanel : public Panel {
	public:
		TileMapViewerPanel();
		~TileMapViewerPanel();

		void onRender(const std::shared_ptr<BatchRenderer>& batchRenderer);
		void setInstance(gbz80_t* pInstance) { mInstance = pInstance; }

	protected:
		virtual void onImGuiRender() override;

	private:
		gbz80_t* mInstance;
		std::shared_ptr<FrameBuffer> mFBO;
		uint32_t mResizeWidth, mResizeHeight;
		bool mNeedResize;
	};

}