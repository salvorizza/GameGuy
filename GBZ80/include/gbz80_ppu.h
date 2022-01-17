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

	typedef struct sprite_data_t {
		uint8_t y;
		uint8_t x;
		uint8_t tile_index;
		uint8_t attributes_flags;
	} sprite_data_t;

	void gbz80_ppu_init(gbz80_ppu_t* ppu, gbz80_t* instance);
	void gbz80_ppu_step(gbz80_ppu_t* ppu, size_t num_cycles_passed);
	void gbz80_ppu_update_stat_register(gbz80_ppu_t* ppu, uint8_t mode, uint8_t ly);

	void gbz80_ppu_draw_background(gbz80_ppu_t* ppu, uint8_t ly);
	void gbz80_ppu_draw_sprites(gbz80_ppu_t* ppu, uint8_t ly);
	uint8_t gbz80_ppu_tilemap_read_tile_index_by_coords(gbz80_ppu_t* ppu, uint8_t tile_x, uint8_t tile_y);
	
	uint8_t gbz80_ppu_get_sprite_color(gbz80_ppu_t* ppu,uint8_t palette_number, uint8_t color_id);
	uint8_t gbz80_ppu_get_bgp_color(gbz80_ppu_t* ppu, uint8_t color_id);
	uint8_t gbz80_ppu_get_palette_color(gbz80_ppu_t* ppu, uint8_t palette, uint8_t color_id);
	void gbz80_ppu_read_tile_pixels_by_line(gbz80_ppu_t* ppu, uint8_t tile_index, uint8_t tile_line, uint8_t pixels[8]);
	sprite_data_t gbz80_ppu_read_sprite(gbz80_ppu_t* ppu, uint8_t sprite_index);
	void gbz80_ppu_gather_oam_sprites_by_line(gbz80_ppu_t* ppu, uint8_t ly, sprite_data_t sprites[10], uint8_t* num_sprites);


#ifdef __cplusplus
}
#endif