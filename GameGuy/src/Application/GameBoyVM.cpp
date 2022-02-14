#include "Application/GameBoyVM.h"

namespace GameGuy {

	GameBoyVM* GameBoyVM::sInstance = nullptr;

	GameBoyVM::GameBoyVM()
		:	mInstance(NULL),
			mBiosPath("commons/roms/gb_bios.bin"),
			mPrevState(VMState::None),
			mState(VMState::None),
			mCurrentlyLoadedCartridge(NULL) {
		
	}

	GameBoyVM::~GameBoyVM() {
		setState(VMState::Stop);
		mAudioManager->Stop();

		if (mCurrentlyLoadedCartridge) {
			gbz80_cartridge_destroy(mCurrentlyLoadedCartridge);
			mCurrentlyLoadedCartridge = NULL;
		}
		gbz80_destroy(mInstance);
	}

	void GameBoyVM::init(AudioPanel* audioPanel)
	{
		mAudioPanel = audioPanel;
		sInstance = this;
		mInstance = gbz80_create();
		gbz80_init(mInstance, mBiosPath);

		gbz80_set_sample_rate(mInstance, 48000);
		std::vector<std::wstring> devices = AudioManager<int16_t>::Enumerate();
		mAudioManager = std::make_shared<AudioManager<int16_t>>(devices[0], 48000, 1, 8, 512);
		mAudioManager->SetUserFunction(sample);
	}

	void GameBoyVM::update()
	{
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_LEFT, GetAsyncKeyState(0x41) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_RIGHT, GetAsyncKeyState(0x44) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_UP, GetAsyncKeyState(0x57) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_DOWN, GetAsyncKeyState(0x53) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_A, GetAsyncKeyState(0x4B) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_B, GetAsyncKeyState(0x4C) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_SELECT, GetAsyncKeyState(0x4E) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_START, GetAsyncKeyState(0x4D) ? 1 : 0);

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

	double GameBoyVM::sample(double dTime)
	{
		if (sInstance->mState == VMState::Run) {
			do {
				if (sInstance->mState == VMState::Stop) {
					break;
				}

				gbz80_clock(sInstance->mInstance);

				if (sInstance->mInstance->cpu.cycles == 0) {
					if (sInstance->mBreakFunction(sInstance->mInstance->cpu.registers.PC)) {
						sInstance->setState(VMState::Pause);
						break;
					}
				}

			} while (sInstance->mInstance->apu.sample_ready == 0);
			
			double sample = sInstance->mInstance->apu.so_1;
			//sInstance->mAudioPanel->addSample(dTime, sample, sample);

			return sample;
		}
		else {
			return 0;
		}
	}
}