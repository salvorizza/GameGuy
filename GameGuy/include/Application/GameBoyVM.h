#pragma once

#include "gbz80.h"
#include "Timer.h"

#include <functional>
#include "Application/AudioManager.h"

#include "Panels/AudioPanel.h"

#include <deque>

namespace GameGuy {

	enum VMState : uint32_t {
		None,
		Start,
		Run,
		Stop,
		Pause
	};

	typedef std::function<bool(uint16_t)> BreakFunction;

	class GameBoyVM {
	public:
		GameBoyVM();
		~GameBoyVM();

		void init(AudioPanel* audioPanel);
		void update();
		void loadRom(const char* romPath);

		inline void setState(VMState state) { mPrevState = (uint32_t)mState; mState = state; }
		inline VMState getState() const { return (VMState)mState.load(); }

		inline void setBreakFunction(const BreakFunction& breakFunction) { mBreakFunction = breakFunction; }

		operator gbz80_t*() { return mInstance; }

	private:
		static double sample(double dTime);
	private:
		gbz80_t* mInstance;
		gbz80_cartridge_t* mCurrentlyLoadedCartridge;

		std::atomic<uint32_t> mState,mPrevState;

		Timer mTimer;
		BreakFunction mBreakFunction;

		std::shared_ptr<AudioManager<int16_t>> mAudioManager;
		const char* mBiosPath;

		static GameBoyVM* sInstance;
		AudioPanel* mAudioPanel;
	};

}