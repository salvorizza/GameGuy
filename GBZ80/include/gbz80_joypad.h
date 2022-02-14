#pragma once


#ifdef __cplusplus
extern "C" {
#endif

	#include "common.h"

	typedef struct gbz80_t gbz80_t;

	typedef enum gbz80_joypad_button_t {
		GBZ80_JOYPAD_BUTTON_RIGHT,
		GBZ80_JOYPAD_BUTTON_LEFT,
		GBZ80_JOYPAD_BUTTON_UP,
		GBZ80_JOYPAD_BUTTON_DOWN,
		GBZ80_JOYPAD_BUTTON_A,
		GBZ80_JOYPAD_BUTTON_B,
		GBZ80_JOYPAD_BUTTON_SELECT,
		GBZ80_JOYPAD_BUTTON_START,
		GBZ80_JOYPAD_BUTTON_MAX
	} gbz80_joypad_button_t;

	typedef struct gbz80_joypad_t {
		uint8_t status[GBZ80_JOYPAD_BUTTON_MAX];
		gbz80_t* instance;
	} gbz80_joypad_t;

	void gbz80_joypad_init(gbz80_joypad_t* joypad, gbz80_t* instance);
	void gbz80_joypad_press_or_release_button(gbz80_joypad_t* joypad, gbz80_joypad_button_t button, uint8_t pressed);

#ifdef __cplusplus
}
#endif