#include "Panels/Panel.h"

#include <imgui.h>

namespace GameGuy {

	Panel::Panel(const std::string& name)
		:	mName(name),
			mOpen(false)
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
			ImGui::Begin(mName.c_str(), &mOpen);
			onImGuiRender();
			ImGui::End();
		}
	}

	void Panel::close()
	{
		mOpen = false;
	}

}