#include "Panels/CPUStatusPanel.h"

#include <imgui.h>

namespace GameGuy {



	CPUStatusPanel::CPUStatusPanel()
		:	Panel("CPU Status", false),
			mInstance(NULL)
	{}

	CPUStatusPanel::~CPUStatusPanel()
	{
	}


	void CPUStatusPanel::onImGuiRender() {
		float availWidth = ImGui::GetContentRegionAvail().x;
		float oneCharSize = ImGui::CalcTextSize("A").x;

		if (ImGui::BeginTable("CPUStatusTable", 3, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable)) {
			ImGui::TableSetupColumn("16 bit Register", ImGuiTableColumnFlags_WidthFixed, availWidth * 0.33f);
			ImGui::TableSetupColumn("Hi", ImGuiTableColumnFlags_WidthFixed, availWidth * 0.33f);
			ImGui::TableSetupColumn("Lo", ImGuiTableColumnFlags_WidthFixed, availWidth * 0.33f);
			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("AF");
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_A));
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", mInstance->cpu.registers.flags);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("BC");
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_B));
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_C));

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("DE");
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_D));
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_E));

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("HL");
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_H));
			ImGui::TableNextColumn();
			ImGui::Text("0x%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_L));

			ImGui::EndTable();
		}

		if (ImGui::BeginTable("CPUStatusTableFlags", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthFixed, oneCharSize * 2);
			ImGui::TableSetupColumn("N", ImGuiTableColumnFlags_WidthFixed, oneCharSize * 2);
			ImGui::TableSetupColumn("H", ImGuiTableColumnFlags_WidthFixed, oneCharSize * 2);
			ImGui::TableSetupColumn("C", ImGuiTableColumnFlags_WidthFixed, oneCharSize * 2);
			ImGui::TableHeadersRow();

			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::Text("%d", gbz80_cpu_get_flag(&mInstance->cpu, GBZ80_FLAG_ZERO));

			ImGui::TableNextColumn();
			ImGui::Text("%d", gbz80_cpu_get_flag(&mInstance->cpu, GBZ80_FLAG_N));

			ImGui::TableNextColumn();
			ImGui::Text("%d", gbz80_cpu_get_flag(&mInstance->cpu, GBZ80_FLAG_H));

			ImGui::TableNextColumn();
			ImGui::Text("%d", gbz80_cpu_get_flag(&mInstance->cpu, GBZ80_FLAG_C));

			ImGui::EndTable();
		}

		ImGui::Text("Current Instruction: %s", mInstance->cpu.current_instruction.disassembled_name);
		ImGui::Text("\tCurrent cycle/Total cycles: %d/%d", mInstance->cpu.current_instruction.num_current_cycle,mInstance->cpu.current_instruction.num_total_cycles);
		ImGui::Text("\tRead Cycle: %d", mInstance->cpu.current_instruction.read_cycle);
		ImGui::Text("\tWrite Cycle: %d", mInstance->cpu.current_instruction.write_cycle);

		ImGui::Text("PC: 0x%04X", mInstance->cpu.registers.PC);
		ImGui::Text("SP: 0x%04X", mInstance->cpu.registers.SP);
		ImGui::Text("Halted: %d", mInstance->cpu.halted);
		ImGui::Text("IME: %d", mInstance->cpu.ime);
		ImGui::Text("Div: 0x%04X", mInstance->cpu.div);
		ImGui::Text("TIMA: 0x%02X", mInstance->cpu.tima);
		ImGui::Text("TMA: 0x%02X", mInstance->cpu.tma);
		static const size_t freqs[] = { 4096,262144,65536,16384 };
		uint8_t freq = common_get8_bit_range(mInstance->cpu.tac, 0, 1);
		uint8_t enable = common_get8_bit(mInstance->cpu.tac, 2);
		ImGui::Text("TAC");
		ImGui::Text("\tEnable: %d", enable);
		ImGui::Text("\tFrequency: %dhz", freqs[freq]);
		ImGui::Text("DMA Enabled: %d", mInstance->cpu.dma_enabled);
		ImGui::Text("\tDMA Next write: 0x%04X", ((uint16_t)mInstance->cpu.dma << 8) | mInstance->cpu.dma_next_write_address);
		ImGui::Text("\tDMA Cycles: %d", mInstance->cpu.dma_cycle_count);
		ImGui::Text("PPU num dots: %d", mInstance->ppu.num_dots);
		ImGui::Text("PPU LY: %d", mInstance->ppu.ly);
	}

}