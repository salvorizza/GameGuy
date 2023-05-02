#include "Panels/DisassemblerPanel.h"

#include "Window/FontAwesome5.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui.h>

namespace GameGuy {



	DisassemblerPanel::DisassemblerPanel()
		:	Panel("Disassembler", false),
			mInstance(NULL),
			mVMInstance(nullptr),
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

		uint8_t bootstrapMode = mInstance->bootstrap_mode;
		mInstance->bootstrap_mode = 1;
		disassemble(mInstructionsBootRom, mInstance->bootstrap_rom, mInstance->bootstrap_rom + BYTE(256));
		mInstance->bootstrap_mode = bootstrapMode;
		for (auto& [address, debugInstruction] : mInstructionsBootRom)
			mInstructionsBootRomKeys.push_back(address);
	}

	void DisassemblerPanel::disassembleCartridge()
	{
		VMState state = mVMInstance->getState();
		mVMInstance->setState(Pause);
		mVMInstance->waitOnLatch();


		mInstructionsCartridgeKeys.clear();
		uint8_t bootstrapMode = mInstance->bootstrap_mode;
		mInstance->bootstrap_mode = 0;
		disassemble(mInstructionsCartridge, mInstance->inserted_cartridge->rom_banks, mInstance->inserted_cartridge->rom_banks + 0xE000);
		mInstance->bootstrap_mode = bootstrapMode;

		for (auto& [address, debugInstruction] : mInstructionsCartridge)
			mInstructionsCartridgeKeys.push_back(address);

		mVMInstance->setState(state);
	}

	bool DisassemblerPanel::breakFunction(uint16_t address)
	{
		auto it = mInstructionsBreaks.find(std::pair<DebugTab, uint16_t>(mDebugTab, address));
		if (it != mInstructionsBreaks.end()) {
			setDebugState(DebugState::Breakpoint);
			mScrollToCurrent = true;
			mCurrent = address;
			return true;
		}
		return false;
	}


	void DisassemblerPanel::onUpdate()
	{
		switch (mDebugState) {
			case DebugState::Start:
				setDebugState(DebugState::Running);
				mVMInstance->setState(VMState::Run);
				//mInstance->cpu.registers.PC = getCurrentInstructionMap().begin()->first;
				break;

			case DebugState::Running:
				mVMInstance->setState(VMState::Run);
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
				if (getCurrentInstructionMap().find(mInstance->cpu.current_instruction.address) != getCurrentInstructionMap().end()) {
					uint16_t prevAddress = mInstance->cpu.current_instruction.address;
					while (mInstance->cpu.current_instruction.address == prevAddress) {
						gbz80_clock(mInstance);
					} 

					mScrollToCurrent = true;
					mCurrent = mInstance->cpu.current_instruction.address;
					setDebugState(DebugState::Breakpoint);
				}
				else {
					setDebugState(DebugState::Stop);
				}
				break;

			case DebugState::Stop:
				setDebugState(DebugState::Idle);
				mVMInstance->setState(VMState::Stop);
				break;
		}
	}

	void DisassemblerPanel::search(const std::string& key)
	{
		mSearchResults.clear();

		if (key.empty()) {
			return;
		}

		auto& instructionMap = getCurrentInstructionMap();
		for (const auto& pair : instructionMap) {
			size_t offset = pair.second.Instruction.find(key);
			if (offset != pair.second.Instruction.npos) {
				mSearchResults[pair.first] = offset;
			}
		}
		mSearchResultsIterator = mSearchResults.begin();
	}

	void DisassemblerPanel::onImGuiRender()
	{

		float availWidth = ImGui::GetContentRegionAvail().x;
		float oneCharSize = ImGui::CalcTextSize("A").x;
		float bulletSize = oneCharSize * 2;
		float lineSize = oneCharSize * 4;
		float addressingSize = oneCharSize * 7;
		float contentCellsWidth = availWidth - (bulletSize + addressingSize + lineSize);

		if (ImGui::Button(ICON_FA_PLAY)) onPlay();
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_STOP)) onStop();
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_STEP_FORWARD)) onStepForward();
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_SYNC)) disassembleCartridge();
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


		ImGui::Text("Search:");
		ImGui::SameLine();
		static char searchBuffer[32];
		if (ImGui::InputText("##search", searchBuffer, 32)) {
			search(searchBuffer);
			if (!mSearchResults.empty()) {
				mScrollToCurrent = true;
				mCurrent = mSearchResultsIterator->first;
			}
		}
		ImVec2 searchTextUISize = ImGui::CalcTextSize(searchBuffer);


		ImGui::SameLine();

		bool condition = mSearchResults.empty();
		if (condition) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.14f, 0.14f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.14f, 0.14f, 0.0f));
		}
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, condition);

		if (ImGui::Button(ICON_FA_ARROW_RIGHT)) {
			mSearchResultsIterator++;
			if (mSearchResultsIterator == mSearchResults.end()) {
				mSearchResultsIterator = mSearchResults.begin();
			}

			mScrollToCurrent = true;
			mCurrent = mSearchResultsIterator->first;
		}

		ImGui::PopItemFlag();

		if (condition) {
			ImGui::PopStyleColor(3);
		}
		

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

			if (mScrollToCurrent) {
				auto it = std::find(keys.begin(), keys.end(), mCurrent);
				size_t val = std::distance(keys.begin(), it);
				clipper.ForceDisplayRangeByIndices(val - 1, val + 1);
			}

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
					auto& it = instructionMap.find(keys[row]);

					uint16_t address = it->first;
					DebugInstruction& debugInstruction = it->second;

					

					ImGui::TableNextRow();
					if (mScrollToCurrent && address == mCurrent) {
						ImGui::SetScrollHereY(0.75);
						mScrollToCurrent = false;
					}

					if (mDebugState != DebugState::Idle && address == mInstance->cpu.current_instruction.address) {
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(230, 100, 120, 125));
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(180, 50, 70, 125));
					}

					ImGui::TableNextColumn();
					ImGui::PushStyleColor(ImGuiCol_Button, debugInstruction.Breakpoint ? ImVec4(1, 0, 0, 1) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, debugInstruction.Breakpoint ? ImVec4(1, 0, 0, 1) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, debugInstruction.Breakpoint ? ImVec4(1, 0, 0, 1) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,50);
					ImGui::PushID(row + 1);
					if (ImGui::Button("",ImVec2(18,18))) {
						debugInstruction.Breakpoint = !debugInstruction.Breakpoint;
						
						if (debugInstruction.Breakpoint) {
							mInstructionsBreaks.emplace(std::pair<DebugTab, uint16_t>(mDebugTab, address), debugInstruction);
						} else {
							mInstructionsBreaks.erase(std::pair<DebugTab, uint16_t>(mDebugTab, address));
						}
					}
					ImGui::PopID();
					ImGui::PopStyleColor(3);
					ImGui::PopStyleVar(2);

					ImGui::TableNextColumn();
					ImGui::Text("%04u", row + 1);

					ImGui::TableNextColumn();
					ImGui::Text("0x%04X", address);

					

					ImGui::TableNextColumn();
					ImVec2 cursorPos = ImGui::GetCursorPos();
					if (strlen(searchBuffer) != 0) {
						auto searchIterator = mSearchResults.find(keys[row]);
						if (searchIterator != mSearchResults.end()) {
							std::string subString = debugInstruction.Instruction.substr(0, searchIterator->second);
							ImVec2 subStringUIOffset = ImGui::CalcTextSize(subString.c_str());
							ImVec2 windowPos = ImGui::GetWindowPos();
							ImVec2 min = ImVec2(windowPos.x + cursorPos.x + subStringUIOffset.x, windowPos.y + cursorPos.y - ImGui::GetScrollY());
							ImVec2 max = ImVec2(windowPos.x + cursorPos.x + subStringUIOffset.x + searchTextUISize.x, windowPos.y + cursorPos.y + searchTextUISize.y - ImGui::GetScrollY());
							
							ImU32 color = IM_COL32(0, 112, 224, 100);
							if (mSearchResultsIterator->first == keys[row]) {
								color = IM_COL32(0, 112, 224, 255);
							}

							ImGui::GetWindowDrawList()->AddRectFilled(min,max, color);
						}
					}
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

		uint16_t pc = mInstance->cpu.registers.PC;

		mInstance->cpu.registers.PC = 0;
		while (&base[mInstance->cpu.registers.PC] <= end) {
			gbz80_instruction_t instruction;
			gbz80_cpu_fetch(&mInstance->cpu, &instruction);
			gbz80_cpu_decode(&mInstance->cpu, &instruction, 1);

			instructionsMap.emplace(instruction.address, instruction.disassembled_name);
		}

		mInstance->cpu.registers.PC = pc;
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