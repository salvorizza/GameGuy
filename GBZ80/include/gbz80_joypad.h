#pragma once


#ifdef __cplusplus
extern "C" {
#endif

	#include "common.h"

	typedef struct gbz80_t gbz80_t;

	typedef enum gbz80_joypad_button_t {
		GBZ80_JOYPAD_BUTTON_RIGHT = 0x10,
		GBZ80_JOYPAD_BUTTON_LEFT = 0x11,
		GBZ80_JOYPAD_BUTTON_UP = 0x12,
		GBZ80_JOYPAD_BUTTON_DOWN = 0x13,
		GBZ80_JOYPAD_BUTTON_A = 0x20,
		GBZ80_JOYPAD_BUTTON_B = 0x21,
		GBZ80_JOYPAD_BUTTON_SELECT = 0x22,
		GBZ80_JOYPAD_BUTTON_START = 0x23
	} gbz80_joypad_button_t;

	typedef struct gbz80_joypad_t {
		gbz80_t* instance;
	} gbz80_joypad_t;

	void gbz80_joypad_init(gbz80_joypad_t* joypad, gbz80_t* instance);
	void gbz80_joypad_press_or_release_button(gbz80_joypad_t* joypad, gbz80_joypad_button_t button, uint8_t release);

#ifdef __cplusplus
}
#endif