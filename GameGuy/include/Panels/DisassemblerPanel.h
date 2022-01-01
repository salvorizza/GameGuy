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

		void disassembleFile(const char* filePath);
		void disassembleInstance();

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

	private:
		void disassemble(uint8_t* base, uint8_t* end);

		void onPlay();
		void onStop();
		void onStepForward();

		void setDebugState(DebugState debugState) { mPrevDebugState = mDebugState; mDebugState = debugState; }

	

		gbz80_t* mInstance;
		std::map<uint16_t,DebugInstruction> mInstructions;
		DebugState mDebugState;
		DebugState mPrevDebugState;
		bool mScrollToCurrent;
	};

}
