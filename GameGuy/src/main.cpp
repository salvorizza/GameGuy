#include "Application/ApplicationManager.h"
#include "Panels/MemoryEditorPanel.h"
#include "Panels/DisassemblerPanel.h"
#include "Panels/CPUStatusPanel.h"
#include "Panels/ViewportPanel.h"

#include "Graphics/BatchRenderer.h"

#include <imgui.h>
#include "gbz80.h"

using namespace GameGuy;


class GameGuyApp : public Application {
public:
	GameGuyApp() 
		:	Application("Game Guy"),
			mGBZ80Instance(NULL)
	{}

	~GameGuyApp() {
		gbz80_destroy(mGBZ80Instance);
	}

	virtual void onSetup() override {
		mGBZ80Instance = gbz80_create();
		gbz80_init(mGBZ80Instance, "commons/roms/gb_bios.bin");
		gbz80_cartridge_t* cartridge = gbz80_cartridge_read_from_file("commons/roms/tetris.gb");
		gbz80_load_cartridge(mGBZ80Instance, cartridge);
		gbz80_cartridge_destroy(cartridge);

		mBatchRenderer = std::make_shared<BatchRenderer>();


		mMemoryEditorPanel.setInstance(mGBZ80Instance);
		mDisassemblerPanel.setInstance(mGBZ80Instance);
		mCPUStatusPanel.setInstance(mGBZ80Instance);
		mViewportPanel.setInstance(mGBZ80Instance);
	}

	virtual void onUpdate() override {
		mDisassemblerPanel.onUpdate();
	}

	virtual void onRender() override {
		mViewportPanel.startFrame();
		glClearColor(1, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		mProjectionMatrix = glm::ortho(0.0f, (float)mViewportPanel.width(), (float)mViewportPanel.height(), 0.0f);
		mBatchRenderer->begin(mProjectionMatrix);

		float cellWidth = (float)mViewportPanel.width() / 160.0f;
		float cellHeight = (float)mViewportPanel.height() / 144.0f;
		for (size_t y = 0; y < 144; y++) {
			for (size_t x = 0; x < 160; x++) {
				uint8_t col = mGBZ80Instance->ppu.lcd[y * 160 + x];
				glm::vec4 color;
				switch (col) {
					case 0: color = { 1,1,1,1 }; break;
					case 1: color = { .82f,.82f,.82f,1 }; break;
					case 2: color = { .37f,.37f,.37f,1 }; break;
					case 3: color = { 0,0,0,1 }; break;
				}

				mBatchRenderer->drawQuad({ x * cellWidth, y * cellHeight }, { cellWidth,cellHeight }, { 1,1,1,1 });
			}
		}

		mBatchRenderer->end();

		mViewportPanel.endFrame();
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
		mViewportPanel.render();
	}

private:
	gbz80_t* mGBZ80Instance;
	MemoryEditorPanel mMemoryEditorPanel;
	DisassemblerPanel mDisassemblerPanel;
	CPUStatusPanel mCPUStatusPanel;
	ViewportPanel mViewportPanel;
	glm::mat4 mProjectionMatrix;

	std::shared_ptr<BatchRenderer> mBatchRenderer;
};

int main(int argc, char** argv) {
	ApplicationManager appManager;
	appManager.run(std::make_shared<GameGuyApp>());
	return 0;
}