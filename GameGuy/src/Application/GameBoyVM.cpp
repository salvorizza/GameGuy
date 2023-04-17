#include "Application/GameBoyVM.h"

#include "Window/InputManager.h"

namespace GameGuy {

	GameBoyVM* GameBoyVM::sInstance = nullptr;

	GameBoyVM::GameBoyVM()
		:	mInstance(NULL),
			mBiosPath("commons/roms/dmg_boot.bin"),
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

	void GameBoyVM::init(const std::shared_ptr<AudioPanel>& audioPanel)
	{
		mAudioPanel = audioPanel;
		sInstance = this;
		mInstance = gbz80_create();
		gbz80_init(mInstance, mBiosPath);
		mRenderingSample = 0;
		memset(mBuffer, 0, sizeof(mBuffer));

		gbz80_set_sample_rate(mInstance, 48000);
		std::vector<std::wstring> devices = AudioManager<int16_t>::Enumerate();
		mAudioManager = std::make_shared<AudioManager<int16_t>>(devices[0], 48000, 1, 8, 512);
		mAudioManager->SetUserFunction(sample);
	}

	void GameBoyVM::update()
	{
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_LEFT, InputManager::IsKeyPressed(GLFW_KEY_LEFT) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_RIGHT, InputManager::IsKeyPressed(GLFW_KEY_RIGHT) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_UP, InputManager::IsKeyPressed(GLFW_KEY_UP) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_DOWN, InputManager::IsKeyPressed(GLFW_KEY_DOWN) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_A, InputManager::IsKeyPressed(GLFW_KEY_Z) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_B, InputManager::IsKeyPressed(GLFW_KEY_X) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_SELECT, InputManager::IsKeyPressed(GLFW_KEY_BACKSPACE) ? 1 : 0);
		gbz80_joypad_press_or_release_button(&mInstance->joypad, GBZ80_JOYPAD_BUTTON_START, InputManager::IsKeyPressed(GLFW_KEY_ENTER) ? 1 : 0);

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
		if (sInstance->getState() == VMState::Run) {
			sInstance->setState(VMState::Stop);
			waitOnLatch();
		}

		if (mCurrentlyLoadedCartridge) {
			gbz80_cartridge_destroy(mCurrentlyLoadedCartridge);
			mCurrentlyLoadedCartridge = NULL;
		}

		gbz80_init(mInstance, mBiosPath);
		mCurrentlyLoadedCartridge = gbz80_cartridge_read_from_file(romPath);
		gbz80_load_cartridge(mInstance, mCurrentlyLoadedCartridge);
	}

	double GameBoyVM::sample(double dTime)
	{
		double sample = 0;
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
			
			sInstance->mRenderingSample++;
			if (sInstance->mInstance->ppu.state == GBZ80_PPU_STATE_VBLANK && sInstance->mInstance->ppu.ly == 144) {
				memcpy(sInstance->mBuffer,sInstance->mInstance->ppu.lcd, sizeof(mBuffer));
				sInstance->mRenderingSample = 0;
			}

			sample = sInstance->mInstance->apu.so_1;
			sInstance->mAudioPanel->addSample(dTime, sample, sample);
		}

		std::unique_lock<std::mutex> lm(sInstance->mMutexLatch);
		sInstance->mLatch.notify_one();

		return sample;
	}
}