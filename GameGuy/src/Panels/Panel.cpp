#include "Panels/Panel.h"

#include <imgui.h>

namespace GameGuy {

	Panel::Panel(const std::string& name, bool hasMenuBar, bool noPadding, bool defaultOpen)
		:	mName(name),
			mOpen(defaultOpen),
			mHasMenuBar(hasMenuBar),
			mNoPadding(noPadding)
	{}

	Panel::~Panel()
	{
	}

	void Panel::open()
	{
		mOpen = true;
	}

	void Panel::render()
	{
		if (mOpen) {
			ImGuiWindowFlags window_flags = 0;
			if (mHasMenuBar) {
				window_flags |= ImGuiWindowFlags_MenuBar;
			}

			if (mNoPadding) {
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			}
			ImGui::Begin(mName.c_str(), &mOpen, window_flags);
			onImGuiRender();
			ImGui::End();
			if (mNoPadding) {
				ImGui::PopStyleVar(1);
			}
		}
	}

	void Panel::close()
	{
		mOpen = false;
	}

}