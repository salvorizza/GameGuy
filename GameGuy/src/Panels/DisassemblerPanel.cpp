#include "Panels/DisassemblerPanel.h"

#include "Window/FontAwesome5.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui.h>

namespace GameGuy {



	DisassemblerPanel::DisassemblerPanel()
		:	Panel("Disassembler", true),
			mInstance(NULL),
			mDebugState(DebugState::Idle),
			mPrevDebugState(DebugState::None),
			mScrollToCurrent(false)
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

	void DisassemblerPanel::onUpdate()
	{
		switch (mDebugState) {
			case DebugState::Start:
				setDebugState(DebugState::Running);
				mInstance->cpu.registers.PC = mInstructions.begin()->first;
				break;

			case DebugState::Running:
				if (mPrevDebugState == DebugState::Breakpoint) {
					gbz80_step(mInstance);
					mPrevDebugState = DebugState::Step;
				}
				if(mInstructions.find(mInstance->cpu.registers.PC) != mInstructions.end()){
					if (mInstructions[mInstance->cpu.registers.PC].Breakpoint)
						setDebugState(DebugState::Breakpoint);
					else
						gbz80_step(mInstance);
				}
				else {
					setDebugState(DebugState::Stop);
				}
				break;

			case DebugState::Step:
				if (mInstructions.find(mInstance->cpu.registers.PC) != mInstructions.end()) {
					gbz80_step(mInstance);
					mScrollToCurrent = true;
					setDebugState(DebugState::Breakpoint);
				}
				else {
					setDebugState(DebugState::Stop);
				}
				break;

			case DebugState::Stop:
				setDebugState(DebugState::Idle);
				break;
		}
	}

	void DisassemblerPanel::onImGuiRender()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open")) {
					disassembleFile("commons/roms/main.obj");
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		float availWidth = ImGui::GetContentRegionAvail().x;
		float oneCharSize = ImGui::CalcTextSize("A").x;
		float bulletSize = oneCharSize * 2;
		float lineSize = oneCharSize * 4;
		float addressingSize = oneCharSize * 7;
		float contentCellsWidth = availWidth - (bulletSize + addressingSize + lineSize);

		
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

		if (ImGui::Button(ICON_FA_PLAY)) onPlay();
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_STOP)) onStop();
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_STEP_FORWARD)) onStepForward();
		ImGui::SameLine();
		switch (mDebugState)
		{
		case GameGuy::DisassemblerPanel::DebugState::Idle:
			ImGui::TextUnformatted("Idle");
			break;
		case GameGuy::DisassemblerPanel::DebugState::Start:
			ImGui::TextUnformatted("Start");
			break;
		case GameGuy::DisassemblerPanel::DebugState::Running:
			ImGui::TextUnformatted("Running");
			break;
		case GameGuy::DisassemblerPanel::DebugState::Breakpoint:
			ImGui::TextUnformatted("Breakpoint");
			break;
		case GameGuy::DisassemblerPanel::DebugState::Step:
			ImGui::TextUnformatted("Step");
			break;
		case GameGuy::DisassemblerPanel::DebugState::Stop:
			ImGui::TextUnformatted("Stop");
			break;
		default:
			break;
		}

		ImGui::PopStyleColor(3);

		ImGui::BeginChild("Disas");
		if (ImGui::BeginTable("Disassembly Table", 4, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, bulletSize);
			ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, lineSize);
			ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, addressingSize);
			ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed, contentCellsWidth);
			ImGui::TableHeadersRow();


			uint32_t i = 1;
			for (auto& [address, debugInstruction] : mInstructions) {
				
				
				ImGui::TableNextRow();
				if (mDebugState != DebugState::Idle &&  address == mInstance->cpu.registers.PC) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(230, 100, 120, 125));
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(180, 50, 70, 125));

					if (mScrollToCurrent) {
						ImGui::SetScrollHereY(0.75);
						mScrollToCurrent = false;
					}
				}


				ImGui::TableNextColumn();
				ImGui::PushStyleColor(ImGuiCol_Text, debugInstruction.Breakpoint ? ImVec4(1, 0, 0, 1) : ImVec4(0.7, 0.7, 0.7, 1.0));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				ImGui::PushID(i);
				if (ImGui::Button(ICON_FA_CIRCLE)) {
					debugInstruction.Breakpoint = !debugInstruction.Breakpoint;
				}
				ImGui::PopID();
				ImGui::PopStyleColor(4);
				ImGui::PopStyleVar(1);

				ImGui::TableNextColumn();
				ImGui::Text("%04u", i);

				ImGui::TableNextColumn();
				ImGui::Text("0x%04X", address);

				ImGui::TableNextColumn();
				ImGui::Text("%s", debugInstruction.Instruction.c_str());

				i++;
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

	void DisassemblerPanel::disassemble(uint8_t* base, uint8_t* end){
		mInstructions.clear();

		while (&mInstance->memory_map[mInstance->cpu.registers.PC] <= end) {
			gbz80_instruction_t instruction;
			gbz80_cpu_fetch(&mInstance->cpu, &instruction);
			gbz80_cpu_decode(&mInstance->cpu, &instruction);

			mInstructions.emplace(instruction.address, instruction.disassembled_name);
		}
	}

	void DisassemblerPanel::onPlay() {
		switch (mDebugState) {
			case DebugState::Idle:
				setDebugState(DebugState::Start);
				break;

			default:
				setDebugState(DebugState::Running);
				break;
		}
	}

	void DisassemblerPanel::onStop() {
		setDebugState(DebugState::Stop);
	}

	void DisassemblerPanel::onStepForward() {
		if (mDebugState == DebugState::Breakpoint) {
			setDebugState(DebugState::Step);
		}
	}

}