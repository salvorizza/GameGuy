#pragma once

#include <gbz80.h>

#include "Panel.h"
#include <deque>
#include <array>
#include <mutex>

namespace GameGuy {

	class AudioPanel : public Panel {
	public:
		AudioPanel();
		~AudioPanel();

		void addSample(size_t time, double ch1, double ch2, double ch3, double ch4);


	protected:
		virtual void onImGuiRender() override;

	private:
		void renderAudioChannel(int channel, ImDrawList* drawList, ImVec2& pos, ImVec2& region);

	private:
		std::deque<std::pair<size_t, std::array<double,4>>> mSamples;
		std::mutex mMutex;
		const size_t MAX_SAMPLES = 512;
	};

}