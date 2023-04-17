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

		void init(const std::shared_ptr<AudioPanel>& audioPanel);
		void update();
		void loadRom(const char* romPath);

		inline void setState(VMState state) { mPrevState = (uint32_t)mState; mState = state; }
		inline VMState getState() const { return (VMState)mState.load(); }
		inline void waitOnLatch() { 
			std::unique_lock<std::mutex> lm(mMutexLatch);
			mLatch.wait(lm);
		}

		inline void setBreakFunction(const BreakFunction& breakFunction) { mBreakFunction = breakFunction; }

		operator gbz80_t*() { return mInstance; }

		gbz80_t* mInstance;
		uint8_t mBuffer[160 * 144];

	private:
		static double sample(double dTime);
	private:
		gbz80_cartridge_t* mCurrentlyLoadedCartridge;

		std::atomic<uint32_t> mState,mPrevState;

		Timer mTimer;
		BreakFunction mBreakFunction;

		std::shared_ptr<AudioManager<int16_t>> mAudioManager;
		const char* mBiosPath;

		static GameBoyVM* sInstance;
		std::shared_ptr<AudioPanel> mAudioPanel;

		uint32_t mRenderingSample;
		std::condition_variable mLatch;
		std::mutex mMutexLatch;

	};

}