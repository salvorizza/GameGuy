#include "Panels/Panel.h"

#include <imgui.h>

namespace GameGuy {

	Panel::Panel(const std::string& name, bool hasMenuBar)
		:	mName(name),
			mOpen(true),
			mHasMenuBar(hasMenuBar)
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
			ImGui::Begin(mName.c_str(), &mOpen, window_flags);
			onImGuiRender();
			ImGui::End();
		}
	}

	void Panel::close()
	{
		mOpen = false;
	}

}