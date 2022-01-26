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
		if(mInstance->inserted_cartridge)
			mem_edit.DrawContents(mInstance->inserted_cartridge->rom_banks, mInstance->inserted_cartridge->rom_banks_size);
	}

}