#include "Application/ApplicationManager.h"
#include "Panels/MemoryEditorPanel.h"
#include "Panels/DisassemblerPanel.h"
#include "Panels/CPUStatusPanel.h"

#include <imgui.h>
#include "gbz80.h"

class GameGuyApp : public GameGuy::Application {
public:
	GameGuyApp() 
		: GameGuy::Application("Game Guy")
	{
		mGBZ80Instance = gbz80_create();
		gbz80_init(mGBZ80Instance, "commons/roms/gb_bios.bin");

		mMemoryEditorPanel.setInstance(mGBZ80Instance);
		mDisassemblerPanel.setInstance(mGBZ80Instance);
		mCPUStatusPanel.setInstance(mGBZ80Instance);
	}

	~GameGuyApp() {
		gbz80_destroy(mGBZ80Instance);
	}

	virtual void onUpdate() override {
		mDisassemblerPanel.onUpdate();
	}

	virtual void onRender() override {
		glClearColor(1, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	virtual void onImGuiRender() override {
		static bool p_open = true;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("GameGuy", &p_open, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Memory Editor", "CTRL+M")) mMemoryEditorPanel.open();
				if (ImGui::MenuItem("Disassembler", "CTRL+D")) mDisassemblerPanel.open();
				if (ImGui::MenuItem("CPU Status", "CTRL+R")) mCPUStatusPanel.open();

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::End();

		mMemoryEditorPanel.render();
		mDisassemblerPanel.render();
		mCPUStatusPanel.render();
	}

private:
	gbz80_t* mGBZ80Instance;
	GameGuy::MemoryEditorPanel mMemoryEditorPanel;
	GameGuy::DisassemblerPanel mDisassemblerPanel;
	GameGuy::CPUStatusPanel mCPUStatusPanel;

};

int main(int argc, char** argv) {
	GameGuy::ApplicationManager appManager;
	appManager.run(std::make_shared<GameGuyApp>());
	return 0;
}