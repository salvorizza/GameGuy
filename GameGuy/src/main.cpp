#include "Application/ApplicationManager.h"
#include "Application/GameBoyVM.h"

#include "Panels/MemoryEditorPanel.h"
#include "Panels/DisassemblerPanel.h"
#include "Panels/CPUStatusPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/TileMapViewerPanel.h"
#include "Panels/AudioPanel.h"

#include "Graphics/BatchRenderer.h"

#include <imgui.h>
#include "gbz80.h"

#include <iostream>


using namespace GameGuy;


class GameGuyApp : public Application {
public:
	GameGuyApp() 
		:	Application("Game Guy"),
			mProjectionMatrix(glm::identity<glm::mat4>())
	{
	}

	~GameGuyApp() {
	}

	virtual void onSetup() override {
		mGameBoyVM.init(&mAudioPanel);
		mBatchRenderer = std::make_shared<BatchRenderer>();

		mTileMapViewerPanel.setInstance(mGameBoyVM);
		mMemoryEditorPanel.setInstance(mGameBoyVM);
		mDisassemblerPanel.setInstance(&mGameBoyVM);
		mCPUStatusPanel.setInstance(mGameBoyVM);
		mViewportPanel.setInstance(mGameBoyVM);

		mGameBoyVM.setBreakFunction(std::bind(&DisassemblerPanel::breakFunction, &mDisassemblerPanel, std::placeholders::_1));
		mDisassemblerPanel.disassembleBootRom();
	}

	virtual void onUpdate() override {
		mGameBoyVM.update();
		mDisassemblerPanel.onUpdate();
	}

	virtual void onRender() override {

		mViewportPanel.startFrame();
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		mProjectionMatrix = glm::ortho(0.0f, (float)mViewportPanel.width(), (float)mViewportPanel.height(), 0.0f);
		mBatchRenderer->begin(mProjectionMatrix);

		float cellWidth = (float)mViewportPanel.width() / 160.0f;
		float cellHeight = (float)mViewportPanel.height() / 144.0f;
		glm::vec4 color;

		for (size_t y = 0; y < 144; y++) {
			for (size_t x = 0; x < 160; x++) {
				uint8_t col = ((gbz80_t*)mGameBoyVM)->ppu.lcd[y * 160 + x];
				switch (col) {
					case 0: color = { 1,1,1,1 }; break;
					case 1: color = { .66f,.66f,.66f,1 }; break;
					case 2: color = { .33f,.33f,.33f,1 }; break;
					case 3: color = { 0,0,0,1 }; break;
				}

				mBatchRenderer->drawQuad({ x * cellWidth, y * cellHeight }, { cellWidth - 1,cellHeight - 1 }, color);
			}
		}

		mBatchRenderer->end();
		mViewportPanel.endFrame();

		mTileMapViewerPanel.onRender(mBatchRenderer);
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
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "CTRL+M")) {
					mGameBoyVM.loadRom("commons/roms/tetris.gb");
					mDisassemblerPanel.disassembleCartridge();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Memory Editor", "CTRL+M")) mMemoryEditorPanel.open();
				if (ImGui::MenuItem("Disassembler", "CTRL+D")) mDisassemblerPanel.open();
				if (ImGui::MenuItem("CPU Status", "CTRL+R")) mCPUStatusPanel.open();
				if (ImGui::MenuItem("Viewport", "CTRL+O")) mViewportPanel.open();
				if (ImGui::MenuItem("Tile Map Viewer", "CTRL+T")) mTileMapViewerPanel.open();


				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Emulation"))
			{
				if (ImGui::MenuItem("Play")) mGameBoyVM.setState(VMState::Start);
				if (ImGui::MenuItem("Stop")) mGameBoyVM.setState(VMState::Stop);
				if (ImGui::MenuItem("Pause")) mGameBoyVM.setState(VMState::Pause);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();

		mMemoryEditorPanel.render();
		mDisassemblerPanel.render();
		mCPUStatusPanel.render();
		mViewportPanel.render();
		mTileMapViewerPanel.render();
		mAudioPanel.render();
	}


private:
	GameBoyVM mGameBoyVM;

	AudioPanel mAudioPanel;
	MemoryEditorPanel mMemoryEditorPanel;
	DisassemblerPanel mDisassemblerPanel;
	CPUStatusPanel mCPUStatusPanel;
	ViewportPanel mViewportPanel;
	TileMapViewerPanel mTileMapViewerPanel;

	Timer mTimer;
	glm::mat4 mProjectionMatrix;

	std::shared_ptr<BatchRenderer> mBatchRenderer;

	
};

int main(int argc, char** argv) {
	ApplicationManager appManager;
	appManager.run(std::make_shared<GameGuyApp>());
	return 0;
}