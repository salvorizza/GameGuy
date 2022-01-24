#pragma once

#include <gbz80.h>

#include "Panel.h"
#include <deque>

namespace GameGuy {

	class AudioPanel : public Panel {
	public:
		AudioPanel();
		~AudioPanel();

		void addSample(size_t time, double left, double right);


	protected:
		virtual void onImGuiRender() override;

	private:
		std::deque<std::tuple<size_t, double, double>> mSamples;
		const size_t MAX_SAMPLES = 48000;
	};

}