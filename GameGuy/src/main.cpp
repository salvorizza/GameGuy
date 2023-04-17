#include "Application/ApplicationManager.h"
#include "Application/GameBoyVM.h"

#include "Panels/MemoryEditorPanel.h"
#include "Panels/DisassemblerPanel.h"
#include "Panels/CPUStatusPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/TileMapViewerPanel.h"
#include "Panels/AudioPanel.h"
#include "Panels/FileDialogPanel.h"

#include "Graphics/BatchRenderer.h"
#include "Window/FontAwesome5.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
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
		mBatchRenderer = std::make_shared<BatchRenderer>();

		mAudioPanel = std::make_shared<AudioPanel>();
		mMemoryEditorPanel = std::make_shared<MemoryEditorPanel>();
		mDisassemblerPanel = std::make_shared<DisassemblerPanel>();
		mCPUStatusPanel = std::make_shared<CPUStatusPanel>();
		mViewportPanel = std::make_shared<ViewportPanel>();
		mTileMapViewerPanel = std::make_shared<TileMapViewerPanel>();
		mFileDialogPanel = std::make_shared<FileDialogPanel>();
		mGameBoyVM.init(mAudioPanel);


		mTileMapViewerPanel->setInstance(mGameBoyVM);
		mMemoryEditorPanel->setInstance(mGameBoyVM);
		mDisassemblerPanel->setInstance(&mGameBoyVM);
		mCPUStatusPanel->setInstance(mGameBoyVM);
		mViewportPanel->setInstance(mGameBoyVM);

		mFileDialogPanel->setCurrentPath("commons/roms");
		mFileDialogPanel->setOnFileSelectedCallback(std::bind(&GameGuyApp::onFileSelected, this, std::placeholders::_1));

		mGameBoyVM.setBreakFunction(std::bind(&DisassemblerPanel::breakFunction, mDisassemblerPanel, std::placeholders::_1));
		mDisassemblerPanel->disassembleBootRom();
	}

	virtual void onUpdate() override {
		mGameBoyVM.update();
		mDisassemblerPanel->onUpdate();
	}

	virtual void onRender() override {
		mViewportPanel->startFrame();
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		mProjectionMatrix = glm::ortho(0.0f, (float)mViewportPanel->width(), (float)mViewportPanel->height(), 0.0f);

		float cellWidth = (float)mViewportPanel->width() / 160.0f;
		float cellHeight = (float)mViewportPanel->height() / 144.0f;
		glm::vec4 color;

		mBatchRenderer->begin(mProjectionMatrix, 0.1f, 1);


		for (size_t y = 0; y < 144; y++) {
			for (size_t x = 0; x < 160; x++) {
				uint8_t col = mGameBoyVM.mBuffer[y * 160 + x];
				switch (col) {
					case 0: color = { 1,1,1,1 }; break;
					case 1: color = { .66f,.66f,.66f,1 }; break;
					case 2: color = { .33f,.33f,.33f,1 }; break;
					case 3: color = { 0,0,0,1 }; break;
				}

				mBatchRenderer->drawQuad({ x * cellWidth, y * cellHeight }, { cellWidth,cellHeight }, color);
			}
		}

		mBatchRenderer->end();
		mViewportPanel->endFrame();

		mTileMapViewerPanel->onRender(mBatchRenderer);
	}

	virtual void onImGuiRender(const std::shared_ptr<ImGuiManager>& pManager, const std::shared_ptr<Window>& pWindow) override {
		static bool p_open = true;

		mFileDialogPanel->setIconForExtension(".*", pManager->LoadIconResource("commons/icons/file.png"), "FILE");
		mFileDialogPanel->setIconForExtension(".gb", pManager->LoadIconResource("commons/icons/rom.png"), "ROM");
		mFileDialogPanel->setIconForExtension(".rom", pManager->LoadIconResource("commons/icons/rom.png"), "ROM");
		mFileDialogPanel->setFolderIcon(pManager->LoadIconResource("commons/icons/folder.png"));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGui::Begin("GameGuy", &p_open, window_flags);
		ImGui::PopStyleVar(2);

		ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImVec2 windowPos = ImGui::GetWindowPos();
		static bool isDragging = false;
		static int startDraggingWindowPosX, startDraggingWindowPosY;
		static ImVec2 borderSize = ImVec2(5,5);

		if (ImGui::BeginMenuBar())
		{
			
			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				isDragging = true;
				pWindow->GetPosition(startDraggingWindowPosX, startDraggingWindowPosY);
			}

			if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				isDragging = false;
			}

			

			ImGui::Button("Logo", ImVec2(60, 60));

			
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "CTRL+M")) {
					mFileDialogPanel->open();
					//mGameBoyVM.loadRom("commons/roms/gb-test-roms-master/instr_timing/instr_timing.gb");
					//mDisassemblerPanel.disassembleCartridge();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Memory Editor", "CTRL+M")) mMemoryEditorPanel->open();
				if (ImGui::MenuItem("Disassembler", "CTRL+D")) mDisassemblerPanel->open();
				if (ImGui::MenuItem("CPU Status", "CTRL+R")) mCPUStatusPanel->open();
				if (ImGui::MenuItem("Viewport", "CTRL+O")) mViewportPanel->open();
				if (ImGui::MenuItem("Tile Map Viewer", "CTRL+T")) mTileMapViewerPanel->open();


				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Emulation"))
			{
				if (ImGui::MenuItem("Play")) mGameBoyVM.setState(VMState::Start);
				if (ImGui::MenuItem("Stop")) mGameBoyVM.setState(VMState::Stop);
				if (ImGui::MenuItem("Pause")) mGameBoyVM.setState(VMState::Pause);

				ImGui::EndMenu();
			}

			float menuMaxSize = ImGui::GetWindowSize().x;
			ImGui::SetCursorPosX(menuMaxSize - 110);

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, -0.5f));
			if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE, ImVec2(30,0))) pWindow->Iconify();
			ImGui::PopStyleVar();

			if (pWindow->isMaximized()) {
				if (ImGui::Button(ICON_FA_WINDOW_RESTORE, ImVec2(30, 0))) pWindow->Restore();
			} else {
				if (ImGui::Button(ICON_FA_SQUARE, ImVec2(30, 0))) pWindow->Maximize();
			}
			if (ImGui::Button(ICON_FA_TIMES, ImVec2(30, 0))) {
				pWindow->Close();
			}

			ImGui::PopStyleColor(3);

			ImGui::EndMenuBar();
		}

		if (isDragging && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			ImVec2 delta = ImGui::GetMouseDragDelta();
			pWindow->SetPosition(startDraggingWindowPosX + delta.x, startDraggingWindowPosY + delta.y);
		}

		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		ImGui::End();


		mMemoryEditorPanel->render(pManager);
		mDisassemblerPanel->render(pManager);
		mCPUStatusPanel->render(pManager);
		mViewportPanel->render(pManager);
		mTileMapViewerPanel->render(pManager);
		mAudioPanel->render(pManager);
		mFileDialogPanel->render(pManager);
	}

	void onFileSelected(const char* filePath) {
		mGameBoyVM.loadRom(filePath);
		mDisassemblerPanel->disassembleCartridge();
		mGameBoyVM.setState(VMState::Start);
	}

private:
	GameBoyVM mGameBoyVM;

	std::shared_ptr<AudioPanel> mAudioPanel;
	std::shared_ptr<MemoryEditorPanel> mMemoryEditorPanel;
	std::shared_ptr<DisassemblerPanel> mDisassemblerPanel;
	std::shared_ptr<CPUStatusPanel> mCPUStatusPanel;
	std::shared_ptr<ViewportPanel> mViewportPanel;
	std::shared_ptr<TileMapViewerPanel> mTileMapViewerPanel;
	std::shared_ptr<FileDialogPanel> mFileDialogPanel;

	Timer mTimer;
	glm::mat4 mProjectionMatrix;

	std::shared_ptr<BatchRenderer> mBatchRenderer;

	
};

int main(int argc, char** argv) {
	ApplicationManager appManager;
	appManager.run(std::make_shared<GameGuyApp>());
	return 0;
}