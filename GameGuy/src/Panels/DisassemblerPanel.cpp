#include "Panels/DisassemblerPanel.h"

#include "Window/FontAwesome5.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui.h>

namespace GameGuy {



	DisassemblerPanel::DisassemblerPanel()
		:	Panel("Disassembler", false),
			mInstance(NULL),
			mDebugState(DebugState::Idle),
			mPrevDebugState(DebugState::None),
			mScrollToCurrent(false),
			mDebugTab(DebugTab::BootRom),
			mSelectTab(DebugTab::None)
	{
	}

	DisassemblerPanel::~DisassemblerPanel()
	{
	}

	void DisassemblerPanel::disassembleBootRom()
	{
		mInstructionsBootRomKeys.clear();
		disassemble(mInstructionsBootRom, mInstance->bootstrap_rom, mInstance->bootstrap_rom + BYTE(256));
		for (auto& [address, debugInstruction] : mInstructionsBootRom)
			mInstructionsBootRomKeys.push_back(address);
	}

	void DisassemblerPanel::disassembleCartridge()
	{
		mInstructionsCartridgeKeys.clear();
		mInstance->bootstrap_mode = 0;
		disassemble(mInstructionsCartridge, mInstance->memory_map, mInstance->rom_bank_0 + mInstance->cartridge_code_size);
		mInstance->bootstrap_mode = 1;
		for (auto& [address, debugInstruction] : mInstructionsCartridge)
			mInstructionsCartridgeKeys.push_back(address);
	}

	bool DisassemblerPanel::breakFunction(uint16_t address)
	{
		uint16_t currentPC = mInstance->cpu.registers.PC;
		auto& instructionMap = getCurrentInstructionMap();
		bool isBreaked = instructionMap.at(currentPC).Breakpoint;
		if (isBreaked) {
			setDebugState(DebugState::Breakpoint);
		}
		return isBreaked;
	}


	void DisassemblerPanel::onUpdate()
	{
		switch (mDebugState) {
			case DebugState::Start:
				setDebugState(DebugState::Running);
				//mInstance->cpu.registers.PC = getCurrentInstructionMap().begin()->first;
				break;

			case DebugState::Running:
				/*if (mPrevDebugState == DebugState::Breakpoint) {
					gbz80_step(mInstance);
					mPrevDebugState = DebugState::Step;
				}
				if(getCurrentInstructionMap().find(mInstance->cpu.registers.PC) != getCurrentInstructionMap().end()){
					if (getCurrentInstructionMap()[mInstance->cpu.registers.PC].Breakpoint)
						setDebugState(DebugState::Breakpoint);
					else {
						for (int i = 0; i < 5000; i++) {
							if (mDebugTab == DebugTab::BootRom && mInstance->bootstrap_mode == 0) {
								mSelectTab = DebugTab::Cartridge;
								break;
							}

							if (getCurrentInstructionMap()[mInstance->cpu.registers.PC].Breakpoint) {
								setDebugState(DebugState::Breakpoint);
								break;
							}
							else {
								gbz80_step(mInstance);
							}
						}
					}
				}
				else {
					setDebugState(DebugState::Stop);
				}*/
				break;

			case DebugState::Step:
				if (getCurrentInstructionMap().find(mInstance->cpu.registers.PC) != getCurrentInstructionMap().end()) {
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

		static bool p_open = true;
		static bool p_open_2 = true;

		if (ImGui::BeginTabBar("SelectDisassembleRom"))
		{
			ImGuiTabItemFlags flag = ImGuiTabItemFlags_None;
			if (mSelectTab == DebugTab::BootRom) {
				flag = ImGuiTabItemFlags_SetSelected;
				mSelectTab = DebugTab::None;
			}

			if (ImGui::BeginTabItem("Bootstrap Rom", &p_open, flag))
			{
				mDebugTab = DebugTab::BootRom;
				ImGui::EndTabItem();
			}

			flag = ImGuiTabItemFlags_None;
			if (mSelectTab == DebugTab::Cartridge) {
				flag = ImGuiTabItemFlags_SetSelected;
				mSelectTab = DebugTab::None;
			}
			if (ImGui::BeginTabItem("Cartridge", &p_open, flag))
			{
				mDebugTab = DebugTab::Cartridge;
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::BeginChild("Disas");
		if (ImGui::BeginTable("Disassembly Table", 4, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, bulletSize);
			ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, lineSize);
			ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, addressingSize);
			ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed, contentCellsWidth);
			ImGui::TableHeadersRow();

		

			uint32_t i = 1;
			auto& instructionMap = getCurrentInstructionMap();
			auto& keys = getCurrentInstructionMapKeys();
			ImGuiListClipper clipper;

			clipper.Begin(instructionMap.size());
			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
					auto& it = instructionMap.find(keys[row]);

					uint16_t address = it->first;
					DebugInstruction& debugInstruction = it->second;

					ImGui::TableNextRow();
					if (mDebugState != DebugState::Idle && address == mInstance->cpu.registers.PC) {
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(230, 100, 120, 125));
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(180, 50, 70, 125));

						if (mScrollToCurrent) {
							ImGui::SetScrollHereY(0.75);
							mScrollToCurrent = false;
						}
					}

					ImGui::TableNextColumn();
					ImGui::PushStyleColor(ImGuiCol_Text, debugInstruction.Breakpoint ? ImVec4(1, 0, 0, 1) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					ImGui::PushID(row);
					if (ImGui::Button(ICON_FA_CIRCLE)) {
						debugInstruction.Breakpoint = !debugInstruction.Breakpoint;
					}
					ImGui::PopID();
					ImGui::PopStyleColor(4);
					ImGui::PopStyleVar(1);

					ImGui::TableNextColumn();
					ImGui::Text("%04u", row + 1);

					ImGui::TableNextColumn();
					ImGui::Text("0x%04X", address);

					ImGui::TableNextColumn();
					ImGui::Text("%s", debugInstruction.Instruction.c_str());
				}
			}
			clipper.End();
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

	void DisassemblerPanel::disassemble(std::map<uint16_t, DisassemblerPanel::DebugInstruction>& instructionsMap,uint8_t* base, uint8_t* end){
		instructionsMap.clear();

		mInstance->cpu.registers.PC = 0;
		while (&base[mInstance->cpu.registers.PC] <= end) {
			gbz80_instruction_t instruction;
			gbz80_cpu_fetch(&mInstance->cpu, &instruction);
			gbz80_cpu_decode(&mInstance->cpu, &instruction, 1);

			instructionsMap.emplace(instruction.address, instruction.disassembled_name);
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