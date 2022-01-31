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

		const char* items[] = { "Cartridge Memory", "Internal Memory"};
		static int item_current = 1;
		ImGui::Combo(" ", &item_current, items, IM_ARRAYSIZE(items));

		if (item_current == 1) {
			mem_edit.DrawContents(mInstance->memory_map, 0xFFFF);
		}
		else {
			if (mInstance->inserted_cartridge) {
				mem_edit.DrawContents(mInstance->inserted_cartridge->rom_banks, mInstance->inserted_cartridge->rom_banks_size);
			}
		}

		/*if (mInstance->inserted_cartridge)
			mem_edit.DrawContents(mInstance->inserted_cartridge->rom_banks, mInstance->inserted_cartridge->rom_banks_size);*/
	}

}