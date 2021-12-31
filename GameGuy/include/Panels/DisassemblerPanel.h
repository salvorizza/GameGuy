#pragma once

#include <Panels/Panel.h>

#include <gbz80.h>

#include <vector>

namespace GameGuy {

	class DisassemblerPanel : public Panel {
	public:
		DisassemblerPanel();
		~DisassemblerPanel();

		void setInstance(gbz80_t* pInstance) { mInstance = pInstance; }

		void disassembleFile(const char* filePath);
		void disassembleInstance();

	protected:
		virtual void onImGuiRender() override;

	private:
		void disassemble(uint8_t* base, uint8_t* end);

	private:
		struct DebugInstruction {
			gbz80_instruction_t* base_instruction;
			bool Breakpoint;

			DebugInstruction(gbz80_instruction_t* instruction, bool breakpoint = false)
				:	Breakpoint(breakpoint),
					base_instruction(instruction)
			{}
		};

		gbz80_t* mInstance;
		std::vector<DebugInstruction> mInstructions;
	};

}
