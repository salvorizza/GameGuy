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

		if (ImGui::BeginTable("CPUStatusTable", 3, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("16 bit Register", ImGuiTableColumnFlags_WidthFixed, availWidth * 0.33);
			ImGui::TableSetupColumn("Hi", ImGuiTableColumnFlags_WidthFixed, availWidth * 0.33);
			ImGui::TableSetupColumn("Lo", ImGuiTableColumnFlags_WidthFixed, availWidth * 0.33);
			ImGui::TableHeadersRow();

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("AF");
			ImGui::TableNextColumn();
			ImGui::Text("%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_A));
			ImGui::TableNextColumn();
			ImGui::Text("%02X", mInstance->cpu.registers.flags);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("BC");
			ImGui::TableNextColumn();
			ImGui::Text("%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_B));
			ImGui::TableNextColumn();
			ImGui::Text("%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_C));

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("DE");
			ImGui::TableNextColumn();
			ImGui::Text("%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_D));
			ImGui::TableNextColumn();
			ImGui::Text("%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_E));

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("HL");
			ImGui::TableNextColumn();
			ImGui::Text("%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_H));
			ImGui::TableNextColumn();
			ImGui::Text("%02X", gbz80_cpu_get_register8(&mInstance->cpu, GBZ80_REGISTER_L));

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

		ImGui::Text("PC: 0x%04X", mInstance->cpu.registers.PC);
		ImGui::Text("SP: 0x%04X", mInstance->cpu.registers.SP);

	}

}