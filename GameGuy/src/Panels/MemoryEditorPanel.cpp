#include "Panels/MemoryEditorPanel.h"

#include <imgui.h>
#include <string>

namespace GameGuy {



	MemoryEditorPanel::MemoryEditorPanel()
		:	Panel("Memory Editor", false),
			mInstance(NULL)
	{}

	MemoryEditorPanel::~MemoryEditorPanel()
	{
	}


	void MemoryEditorPanel::onImGuiRender()
	{
		bool searchAddress = false;
		static char addressSearch[] = "0x0000";
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Address: ");
		ImGui::SameLine();
		searchAddress = ImGui::InputText(" ", addressSearch, 7);
		int32_t addressSearchInt;
		sscanf(addressSearch, "0x%04X", &addressSearchInt);
		sprintf(addressSearch, "0x%04X", addressSearchInt);

		ImGui::BeginChild("Memmory_Editor_Table");

		float availWidth = ImGui::GetContentRegionAvail().x;
		float oneCharSize = ImGui::CalcTextSize("A").x;
		float addressingSize = oneCharSize * 7;
		float cellSize = oneCharSize * 2;
		float contentCellsWidth = availWidth - addressingSize;
		float numCellsFl = contentCellsWidth / (cellSize + 8);
		int32_t numCells = floor(numCellsFl);

		uint32_t numRows = 0;
		if (numCells >= 0) {
			numRows = round((float)0x10000 / numCells);
		}
		else {
			numRows = 0;
			numCells = 0;
		}

		if (ImGui::BeginTable("Memory Table", numCells + 1, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersOuter)) {
			ImGui::TableSetupColumn(" ", ImGuiTableColumnFlags_WidthFixed, addressingSize);
			for (uint32_t column = 0; column < numCells; column++) {
				static char hex[3];
				sprintf(hex, "%02X", column);
				ImGui::TableSetupColumn(hex, ImGuiTableColumnFlags_WidthFixed, cellSize);
			}
			ImGui::TableHeadersRow();

			for (uint32_t row = 0; row < numRows; row++)
			{
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("0x%04X", row * numCells);

				for (uint32_t column = 0; column < numCells; column++) {
					ImGui::TableSetColumnIndex(column + 1);

					uint32_t addressing = row * numCells + column;
					if (addressing >= 0x0000 && addressing <= 0xFFFF) {
						uint8_t val = mInstance->memory_map[addressing];
						ImGui::Text("%02X", val);

						if (searchAddress && addressSearchInt == addressing) {
							ImGui::SetScrollHereY(0);
						}
					}
					else {
						ImGui::Text("");
					}
				}
				
			}

			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

}