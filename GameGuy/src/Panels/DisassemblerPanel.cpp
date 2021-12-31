#include "Panels/DisassemblerPanel.h"

#include "Window/FontAwesome5.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui.h>

namespace GameGuy {



	DisassemblerPanel::DisassemblerPanel()
		:	Panel("Disassembler", true),
			mInstance(NULL)
	{
	}

	DisassemblerPanel::~DisassemblerPanel()
	{
	}

	void DisassemblerPanel::disassembleFile(const char* filePath)
	{
		FILE* f = fopen(filePath, "rb");
		if (f) {
			fseek(f, 0, SEEK_END);
			size_t codeSize = ftell(f);
			fseek(f, 0, SEEK_SET);

			fread(mInstance->memory_map, codeSize, 1, f);
			fclose(f);

			mInstance->cpu.registers.PC = 0x0000;
			disassemble(mInstance->memory_map, mInstance->memory_map + codeSize);
		}
	}

	void DisassemblerPanel::disassembleInstance()
	{
		disassemble(mInstance->rom_bank_0, mInstance->rom_bank_0 + mInstance->cartridge_code_size);
	}

	void DisassemblerPanel::onImGuiRender()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open")) {
					disassembleFile("commons/roms/gb_bios.bin");
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		float availWidth = ImGui::GetContentRegionAvail().x;
		float oneCharSize = ImGui::CalcTextSize("A").x;
		float bulletSize = oneCharSize;
		float lineSize = oneCharSize * 4;
		float addressingSize = oneCharSize * 7;
		float contentCellsWidth = availWidth - (bulletSize + addressingSize + lineSize);

		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

		ImGui::Button(ICON_FA_PLAY);
		ImGui::SameLine();
		ImGui::Button(ICON_FA_STOP);
		ImGui::SameLine();
		ImGui::Button(ICON_FA_STEP_FORWARD);

		ImGui::PopStyleColor(3);

		ImGui::BeginChild("Disas");
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		if (ImGui::BeginTable("Disassembly Table", 4, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, bulletSize);
			ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, lineSize);
			ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, addressingSize);
			ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed, contentCellsWidth);
			ImGui::TableHeadersRow();

			uint32_t i = 1;
			for (DebugInstruction& debugInstruction : mInstructions) {
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				ImVec2 minR = ImGui::GetCursorScreenPos();
				ImVec2 maxR = ImGui::GetCursorScreenPos() + ImVec2(oneCharSize * 2, oneCharSize * 2);
				ImVec2 center = minR + ImVec2(oneCharSize / 2, oneCharSize);

				if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(minR,maxR)){
					debugInstruction.Breakpoint = !debugInstruction.Breakpoint;
				}

				if(debugInstruction.Breakpoint)
					draw_list->AddCircleFilled(center, oneCharSize / 2,ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), 20);
				else
					draw_list->AddCircleFilled(center, oneCharSize / 2, ImColor(ImVec4(0.7f, 0.7f, 0.7f, 1.0f)), 20);

				ImGui::TableNextColumn();
				ImGui::Text("%04u", i);

				ImGui::TableNextColumn();
				ImGui::Text("0x%04X", debugInstruction.base_instruction->address);

				ImGui::TableNextColumn();
				ImGui::Text("%s", debugInstruction.base_instruction->disassembled_name);

				i++;
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

	void DisassemblerPanel::disassemble(uint8_t* base, uint8_t* end){
		for (const DebugInstruction& debugInstruction : mInstructions) {
			delete debugInstruction.base_instruction;
		}
		mInstructions.clear();

		while (&mInstance->memory_map[mInstance->cpu.registers.PC] <= end) {
			gbz80_instruction_t* instruction = (gbz80_instruction_t*)malloc(sizeof(gbz80_instruction_t));
			gbz80_cpu_fetch(&mInstance->cpu, instruction);
			gbz80_cpu_decode(&mInstance->cpu, instruction);

			mInstructions.emplace_back(instruction);
		}
	}

}