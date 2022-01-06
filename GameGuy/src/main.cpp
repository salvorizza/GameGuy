#include "Application/ApplicationManager.h"
#include "Panels/MemoryEditorPanel.h"
#include "Panels/DisassemblerPanel.h"
#include "Panels/CPUStatusPanel.h"
#include "Panels/ViewportPanel.h"

#include "Graphics/VertexArray.h"
#include "Graphics/FrameBuffer.h"

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

		float data[] = {
			0.0f,0.5f,1.0f,0.0f,0.0f,1.0f,
			0.5f,-0.5f,0.0f,1.0f,0.0f,1.0f,
			-0.5f,-0.5f,0.0f,0.0f,1.0f,1.0f
		};

		uint32_t indices[] = {
			0,1,2
		};

		mVBO = std::make_shared<VertexBuffer>();
		mIBO = std::make_shared<IndexBuffer>(indices,sizeof(indices) / sizeof(uint32_t));
		mVBO->setLayout({
				BufferElement("Position", ShaderType::Float2),
				BufferElement("Color", ShaderType::Float4)
		});
		mVBO->setData(data, sizeof(data), VertexBufferDataUsage::Dynamic);

		mVAO = std::make_shared<VertexArray>();
		mVAO->addVertexBuffer(mVBO);
		mVAO->setIndexBuffer(mIBO);

		mMemoryEditorPanel.setInstance(mGBZ80Instance);
		mDisassemblerPanel.setInstance(mGBZ80Instance);
		mCPUStatusPanel.setInstance(mGBZ80Instance);
		mViewportPanel.setInstance(mGBZ80Instance);
	}

	virtual void onUpdate() override {
		mDisassemblerPanel.onUpdate();
	}

	virtual void onRender() override {
		mViewportPanel.onRender();
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

	std::shared_ptr<VertexArray> mVAO;
	std::shared_ptr<VertexBuffer> mVBO;
	std::shared_ptr<IndexBuffer> mIBO;

};

int main(int argc, char** argv) {
	ApplicationManager appManager;
	appManager.run(std::make_shared<GameGuyApp>());
	return 0;
}