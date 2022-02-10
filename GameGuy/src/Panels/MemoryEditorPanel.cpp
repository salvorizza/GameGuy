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

		const char* items[] = { "Internal ROM + RAM","Cartridge ROM","Cartridge RAM"};
		static int item_current = 0;
		ImGui::Combo(" ", &item_current, items, IM_ARRAYSIZE(items));

		switch (item_current) {
			case 0:
				mem_edit.DrawContents(mInstance->memory_map, 0x10000);
				break;
			case 1:
				mem_edit.DrawContents(mInstance->inserted_cartridge->rom_banks, mInstance->inserted_cartridge->rom_banks_size);
				break;
			case 2:
				mem_edit.DrawContents(mInstance->inserted_cartridge->ram_banks, mInstance->inserted_cartridge->ram_banks_size);
				break;
		}
	}

}