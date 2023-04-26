#pragma once

#include <Panels/Panel.h>
#include <Application/GameBoyVM.h>

#include <gbz80.h>

#include <map>
#include <vector>

namespace GameGuy {

	class DisassemblerPanel : public Panel {
	public:
		DisassemblerPanel();
		~DisassemblerPanel();

		void setInstance(GameBoyVM* pInstance) { mVMInstance = pInstance; mInstance = pInstance->mInstance; }

		void disassembleBootRom();
		void disassembleCartridge();

		bool breakFunction(uint16_t address);

		void onUpdate();

	private:
		void search(const std::string& key);

	protected:
		virtual void onImGuiRender() override;

	private:
		struct DebugInstruction {
			std::string Instruction;
			bool Breakpoint;

			DebugInstruction()
				: Breakpoint(false),
				Instruction()
			{}

			DebugInstruction(const std::string& instruction)
				: Breakpoint(false),
				Instruction(instruction)
			{}
		};

		enum class DebugState {
			None,
			Idle,
			Start,
			Running,
			Breakpoint,
			Step,
			Stop
		};

		enum class DebugTab {
			None,
			BootRom,
			Cartridge
		};

	private:
		void disassemble(std::map<uint16_t, DisassemblerPanel::DebugInstruction>& instructionMap, uint8_t* base, uint8_t* end);
		inline std::map<uint16_t, DisassemblerPanel::DebugInstruction>& getCurrentInstructionMap() { return (mDebugTab == DebugTab::BootRom) ? mInstructionsBootRom : mInstructionsCartridge; }
		inline std::vector<uint16_t>& getCurrentInstructionMapKeys() { return (mDebugTab == DebugTab::BootRom) ? mInstructionsBootRomKeys : mInstructionsCartridgeKeys; }

		void onPlay();
		void onStop();
		void onStepForward();

		void setDebugState(DebugState debugState) { mPrevDebugState = mDebugState; mDebugState = debugState; }

		GameBoyVM* mVMInstance;
		gbz80_t* mInstance;

		std::map<std::pair<DebugTab,uint16_t>, DisassemblerPanel::DebugInstruction> mInstructionsBreaks;


		std::map<uint16_t, DisassemblerPanel::DebugInstruction> mInstructionsCartridge;
		std::map<uint16_t, DisassemblerPanel::DebugInstruction> mInstructionsBootRom;
		std::vector<uint16_t> mInstructionsCartridgeKeys;
		std::vector<uint16_t> mInstructionsBootRomKeys;

		std::map<uint16_t, size_t> mSearchResults;
		std::map<uint16_t, size_t>::iterator mSearchResultsIterator;

		DebugState mDebugState;
		DebugState mPrevDebugState;
		DebugTab mDebugTab,mSelectTab;

		bool mScrollToCurrent;
		uint16_t mCurrent;
	};

}
