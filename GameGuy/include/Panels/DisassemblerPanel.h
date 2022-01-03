#pragma once

#include <Panels/Panel.h>

#include <gbz80.h>

#include <map>

namespace GameGuy {

	class DisassemblerPanel : public Panel {
	public:
		DisassemblerPanel();
		~DisassemblerPanel();

		void setInstance(gbz80_t* pInstance) { mInstance = pInstance; }

		void disassembleBootRom();
		void disassembleCartridge();

		void onUpdate();

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
			BootRom,
			Cartridge
		};

	private:
		void disassemble(std::map<uint16_t, DisassemblerPanel::DebugInstruction>& instructionMap, uint8_t* base, uint8_t* end);
		std::map<uint16_t, DisassemblerPanel::DebugInstruction>& getCurrentInstructionMap() { return (mDebugTab == DebugTab::BootRom) ? mInstructionsBootRom : mInstructionsCartridge; }

		void onPlay();
		void onStop();
		void onStepForward();

		void setDebugState(DebugState debugState) { mPrevDebugState = mDebugState; mDebugState = debugState; }

		gbz80_t* mInstance;
		std::map<uint16_t, DisassemblerPanel::DebugInstruction> mInstructionsCartridge;
		std::map<uint16_t, DisassemblerPanel::DebugInstruction> mInstructionsBootRom;

		DebugState mDebugState;
		DebugState mPrevDebugState;
		DebugTab mDebugTab;

		bool mScrollToCurrent;
	};

}
