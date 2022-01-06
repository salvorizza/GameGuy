#pragma once

#include <gbz80.h>
#include <memory>

#include "Panel.h"
#include "Graphics/FrameBuffer.h"

namespace GameGuy {

	class ViewportPanel : public Panel {
	public:
		ViewportPanel();
		~ViewportPanel();

		void setInstance(gbz80_t* pInstance) { mInstance = pInstance; }

		void startFrame();
		void endFrame();

		uint32_t width() const { return mFBO ? mFBO->width() : 0; }
		uint32_t height() const { return mFBO ? mFBO->height() : 0; }

	protected:
		virtual void onImGuiRender() override;

	private:
		gbz80_t* mInstance;
		std::shared_ptr<FrameBuffer> mFBO;
		uint32_t mResizeWidth, mResizeHeight;
		bool mNeedResize;
	};

}