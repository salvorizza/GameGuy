#include "gbz80_joypad.h"

#include "gbz80.h"

void gbz80_joypad_init(gbz80_joypad_t* joypad, gbz80_t* instance) {
	joypad->instance = instance;
	gbz80_memory_write_internal(joypad->instance, 0xFF00, 0xFF);
	memset(joypad->status, 0, sizeof(uint8_t) * GBZ80_JOYPAD_BUTTON_MAX);
}

void gbz80_joypad_press_or_release_button(gbz80_joypad_t* joypad, gbz80_joypad_button_t button, uint8_t pressed) {
	joypad->status[(uint8_t)button] = pressed;
}
