#pragma once

#include <gbz80.h>

#include "Panel.h"

namespace GameGuy {

	class MemoryEditorPanel : public Panel {
	public:
		MemoryEditorPanel();
		~MemoryEditorPanel();

		void setInstance(gbz80_t* pInstance) { mInstance = pInstance; }

	protected:
		virtual void onImGuiRender() override;

	private:
		gbz80_t* mInstance;
	};

}