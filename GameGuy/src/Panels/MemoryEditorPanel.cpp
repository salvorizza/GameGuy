#include "Panels/MemoryEditorPanel.h"

#include <imgui.h>
#include <string>
#include "Panels/MemoryEditor.h"

namespace GameGuy {



	MemoryEditorPanel::MemoryEditorPanel()
		:	Panel("Memory Editor", false),
			mInstance(NULL)
	{}

	MemoryEditorPanel::~MemoryEditorPanel()
	{
	}


	void MemoryEditorPanel::onImGuiRender() {
		static MemoryEditor mem_edit;
		mem_edit.DrawContents(mInstance->memory_map, 0x10000);
	}

}