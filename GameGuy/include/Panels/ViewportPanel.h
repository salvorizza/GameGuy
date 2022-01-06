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

		void onRender();

	protected:
		virtual void onImGuiRender() override;

	private:
		gbz80_t* mInstance;
		std::shared_ptr<FrameBuffer> mFBO;
	};

}