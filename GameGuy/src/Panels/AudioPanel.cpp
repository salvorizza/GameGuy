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

	void AudioPanel::addSample(size_t time, double ch1, double ch2, double ch3, double ch4)
	{
		std::lock_guard<std::mutex> lc(mMutex);

		if (mSamples.size() > MAX_SAMPLES)
			mSamples.pop_front();

		std::array<double, 4> arr = {ch1,ch2,ch3,ch4};
		mSamples.push_back(std::make_pair(time, arr));
	}


	void AudioPanel::onImGuiRender() {
		std::lock_guard<std::mutex> lc(mMutex);

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 region = ImGui::GetContentRegionAvail();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		float yOffset = region.y / 5;

		for(int channel = 0;channel < 4;channel++) {
			ImVec2 channelPos = ImVec2(pos.x, pos.y + (yOffset * (channel + 1)));
			ImVec2 channelRegion = ImVec2(region.x, yOffset);
			
			renderAudioChannel(channel, drawList, channelPos, channelRegion);
		}
	}

	void AudioPanel::renderAudioChannel(int channel, ImDrawList* drawList, ImVec2& pos, ImVec2& region)
	{
		float xInc = region.x / MAX_SAMPLES;
		float amp = region.y / 3;

		ImVec2 previousPos = { pos.x,pos.y + region.y / 2.0f };
		for (auto& sample : mSamples) {
			ImVec2 currentPos;

			currentPos.y = pos.y + (region.y / 2) - (sample.second[channel] * amp);
			currentPos.x = previousPos.x + xInc;

			drawList->AddLine(previousPos, currentPos, ImColor(255, 255 , 255, 255));

			previousPos = currentPos;
		}

		drawList->AddLine({ pos.x,pos.y + region.y / 2.0f }, { pos.x + region.x,pos.y + region.y / 2.0f }, ImColor(255, 255, 255, 75));
	}

}