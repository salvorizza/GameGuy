#include "Application/GameBoyVM.h"

namespace GameGuy {

	

	GameBoyVM::GameBoyVM()
		:	mInstance(NULL),
			mBiosPath("commons/roms/gb_bios.bin"),
			mPrevState(VMState::None),
			mState(VMState::None),
			mCurrentlyLoadedCartridge(NULL)
	{
		mInstance = gbz80_create();
		gbz80_init(mInstance, mBiosPath);
	}

	GameBoyVM::~GameBoyVM()
	{
		if (mCurrentlyLoadedCartridge) {
			gbz80_cartridge_destroy(mCurrentlyLoadedCartridge);
			mCurrentlyLoadedCartridge = NULL;
		}
		gbz80_destroy(mInstance);
	}

	void GameBoyVM::update()
	{
		mTimer.update();
		switch (mState)
		{
			case GameGuy::VMState::Start:
				if (mPrevState != VMState::Pause) {
					gbz80_init(mInstance, mBiosPath);
					gbz80_load_cartridge(mInstance, mCurrentlyLoadedCartridge);
					mTimer.init();
				}
				setState(VMState::Run);
				break;

			case GameGuy::VMState::Run: {
				size_t cyclesToPass = gbz80_utility_get_num_cycles_from_seconds(mInstance, mTimer.getDeltaTimeInSeconds());
				for (size_t cyclesPassed = 0; cyclesPassed < cyclesToPass;) {
					/*if (mBreakFunction(mInstance->cpu.registers.PC)) {
						setState(VMState::Pause);
						break;
					}*/
					cyclesPassed += gbz80_step(mInstance);
				}
				break;
			}

		case GameGuy::VMState::Stop:
			break;

		case GameGuy::VMState::Pause:
			break;
		}
	}
	void GameBoyVM::loadRom(const char* romPath)
	{
		if (mCurrentlyLoadedCartridge) {
			gbz80_cartridge_destroy(mCurrentlyLoadedCartridge);
			mCurrentlyLoadedCartridge = NULL;
		}
		mCurrentlyLoadedCartridge = gbz80_cartridge_read_from_file(romPath);
		gbz80_load_cartridge(mInstance, mCurrentlyLoadedCartridge);
	}
}