#pragma once

#include "common.h"

#define GBZ80_LCD_SIZE 160 * 144

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct gbz80_t gbz80_t;

	typedef struct gbz80_ppu_t {
		uint8_t lcd[GBZ80_LCD_SIZE];
		uint32_t num_dots;
		gbz80_t* instance;
	} gbz80_ppu_t;

	void gbz80_ppu_init(gbz80_ppu_t* ppu, gbz80_t* instance);
	void gbz80_ppu_step(gbz80_ppu_t* ppu, size_t num_cycles_passed);
	void gbz80_ppu_update_stat_register(gbz80_ppu_t* ppu, uint8_t mode, uint8_t ly);

	void gbz80_ppu_draw_background(gbz80_ppu_t* ppu, uint8_t ly);

#ifdef __cplusplus
}
#endif