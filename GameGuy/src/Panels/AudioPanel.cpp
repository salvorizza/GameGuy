#include "Panels/AudioPanel.h"

#include <imgui.h>

namespace GameGuy {

	AudioPanel::AudioPanel()
		: Panel("Audio Panel ", false, true)
	{
	}

	AudioPanel::~AudioPanel()
	{
	}

	void AudioPanel::addSample(size_t time, double left, double right)
	{
		if (mSamples.size() > MAX_SAMPLES)
			mSamples.pop_front();

		mSamples.push_back(std::make_tuple(time, left, right));
	}


	void AudioPanel::onImGuiRender() {
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 region = ImGui::GetContentRegionAvail();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		float xInc = region.x / MAX_SAMPLES;
		float amp = region.y / 4;


		ImVec2 previousPos = { pos.x,pos.y + region.y / 2.0f };
		for (auto& sample : mSamples) {
			ImVec2 currentPos;

			currentPos.y = pos.y + (region.y / 2) + (std::get<1>(sample) * amp);
			currentPos.x = previousPos.x + xInc;

			drawList->AddLine(previousPos, currentPos, ImColor(255,0,0));

			previousPos = currentPos;
		}

		drawList->AddLine({ pos.x,pos.y + region.y / 2.0f }, { pos.x + region.x,pos.y + region.y / 2.0f }, ImColor(255,255,255, 75));
		//drawList->AddRectFilled({ 10,10 }, { 40,40 }, ImColor(255, 255, 255));

	}

}