#pragma once

#include "gbz80.h"
#include "Timer.h"

#include <functional>

namespace GameGuy {

	enum class VMState {
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

		void update();
		void loadRom(const char* romPath);

		inline void setState(VMState state) { mPrevState = mState; mState = state; }
		inline VMState getState() const { return mState; }

		inline void setBreakFunction(const BreakFunction& breakFunction) { mBreakFunction = breakFunction; }

		operator gbz80_t*() { return mInstance; }
	private:
		gbz80_t* mInstance;
		gbz80_cartridge_t* mCurrentlyLoadedCartridge;
		VMState mState,mPrevState;
		Timer mTimer;
		BreakFunction mBreakFunction;
		const char* mBiosPath;
	};

}