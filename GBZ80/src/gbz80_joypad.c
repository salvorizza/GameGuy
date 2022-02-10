#include "gbz80_joypad.h"

#include "gbz80.h"

void gbz80_joypad_init(gbz80_joypad_t* joypad, gbz80_t* instance) {
	joypad->instance = instance;
	gbz80_memory_write_internal(joypad->instance, 0xFF00, 0xFF);
}

void gbz80_joypad_press_or_release_button(gbz80_joypad_t* joypad, gbz80_joypad_button_t button, uint8_t release) {
	uint8_t jp_register = gbz80_memory_read_internal(joypad->instance, 0xFF00);

	uint8_t select_mask = button & 0xF0;
	uint8_t button_bit = button & 0x0F;

	if ((jp_register & select_mask) == 0) {
		if (release == 0 && common_get8_bit(jp_register, button_bit) == 1) {
			gbz80_cpu_request_interrupt(&joypad->instance->cpu, GBZ80_INTERRUPT_JOYPAD);
		}

		common_change8_bit(&jp_register, button_bit, release);
	}

	gbz80_memory_write_internal(joypad->instance, 0xFF00, jp_register);
}
