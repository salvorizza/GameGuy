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
				if (sInstance->mBreakFunction(sInstance->mInstance->cpu.registers.PC)) {
					sInstance->setState(VMState::Pause);
					break;
				}
				else {
					gbz80_clock(sInstance->mInstance);
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