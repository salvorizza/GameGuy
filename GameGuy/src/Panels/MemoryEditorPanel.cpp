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

		if (ImGui::BeginCombo("##combo", items[item_current])) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (item_current == n); // You can store your selection however you want, outside or inside your objects

				ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;
				switch (n) {
					case 0:
						flags |= mInstance->memory_map == NULL ? ImGuiSelectableFlags_Disabled : 0;
						break;

					case 1:
						if(mInstance->inserted_cartridge == NULL || mInstance->inserted_cartridge->rom_banks == NULL)
							flags |= ImGuiSelectableFlags_Disabled;
						break;

					case 2:
						if (mInstance->inserted_cartridge == NULL || mInstance->inserted_cartridge->ram_banks == NULL)
							flags |= ImGuiSelectableFlags_Disabled;
						break;
				}

				if (ImGui::Selectable(items[n], is_selected, flags))
					item_current = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}

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