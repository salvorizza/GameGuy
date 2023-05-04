#pragma once

#include "common.h"

#define GBZ80_LCD_SIZE 160 * 144

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct gbz80_t gbz80_t;

	typedef struct gbz80_ppu_fifo_element_t {
		uint8_t palette;
		uint8_t pixel_value;
		uint8_t sprite;
	} gbz80_ppu_fifo_element_t;

	typedef struct gbz80_ppu_fifo_t {
		uint8_t size;
		gbz80_ppu_fifo_element_t data[16];
		uint8_t write_index;
		uint8_t read_index;
		uint8_t stopped;
	} gbz80_ppu_fifo_t;

	typedef enum gbz80_ppu_fifo_fetcher_state_t {
		GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_ID,
		GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_LOW,
		GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_HIGH,
		GBZ80_PPU_FIFO_FETCHER_STATE_PUSH,
		GBZ80_PPU_FIFO_FETCHER_STATE_IDLE
	} gbz80_ppu_fifo_fetcher_state_t;

	typedef struct gbz80_ppu_sprite_data_t {
		uint8_t y;
		uint8_t x;
		uint8_t tile_index;
		uint8_t attributes_flags;
	} gbz80_ppu_sprite_data_t;

	typedef struct gbz80_ppu_fifo_fetcher_t {

		gbz80_ppu_fifo_fetcher_state_t state;
		uint16_t tile_address;
		uint8_t tile_index;
		uint8_t tile_high;
		uint8_t tile_low;
		uint8_t fetch_x, fetch_y;
		uint8_t tile_x;

		gbz80_ppu_fifo_fetcher_state_t sprite_fetch_stored_state;
		uint8_t sprite_tile_index;
		uint8_t sprite_tile_high;
		uint8_t sprite_tile_low;
		uint8_t sprite_fetch_x;
		uint8_t sprite_fetch_y;

		gbz80_ppu_sprite_data_t* oam_sprite;
		uint8_t sprite_fetch;
		uint8_t window_fetch;
		uint8_t num_clocks;


		gbz80_ppu_fifo_t fifo;
	} gbz80_ppu_fifo_fetcher_t;


	typedef enum gbz80_ppu_state_t {
		GBZ80_PPU_STATE_HMODE = 0,
		GBZ80_PPU_STATE_VBLANK = 1,
		GBZ80_PPU_STATE_OAM_SCAN = 2,
		GBZ80_PPU_STATE_DRAWING_PIXELS = 3
	} gbz80_ppu_state_t;

	typedef struct gbz80_ppu_t {
		uint8_t lcd[GBZ80_LCD_SIZE];
		uint8_t lcd_x;
		gbz80_ppu_fifo_fetcher_t fifo_fetcher;
		gbz80_ppu_sprite_data_t oam_sprites[10];
		uint8_t num_oam_sprites;
		gbz80_ppu_state_t state;

		uint8_t stat_line;
		uint8_t prev_stat_line_or;

		uint8_t ly, lyc;
		uint8_t lcdc;
		uint8_t lcd_status;
		uint8_t scx, scy;
		uint8_t wx, wy;
		uint8_t bgp;
		uint8_t obp[2];

		uint8_t mode3_delay;

		uint8_t scx_dec;
		uint32_t num_dots;

		uint8_t vram_blocked;
		uint8_t oam_blocked;


		gbz80_t* instance;
	} gbz80_ppu_t;

	typedef enum gbz80_ppu_tilemap_type_t {
		GBZ80_PPU_TILEMAP_0,
		GBZ80_PPU_TILEMAP_1
	} gbz80_ppu_tilemap_type_t;

	void gbz80_ppu_init(gbz80_ppu_t* ppu, gbz80_t* instance);
	void gbz80_ppu_clock(gbz80_ppu_t* ppu);
	void gbz80_ppu_update_stat_register(gbz80_ppu_t* ppu, uint8_t mode);
	void gbz80_ppu_memory_read(gbz80_ppu_t* ppu, uint16_t address, uint8_t* val);
	uint8_t gbz80_ppu_memory_write(gbz80_ppu_t* ppu, uint16_t address, uint8_t current_value, uint8_t* val);
	uint8_t gbz80_ppu_tilemap_read_tile_index_by_coords(gbz80_ppu_t* ppu, uint8_t tile_x, uint8_t tile_y, gbz80_ppu_tilemap_type_t map_type);
	
	uint8_t gbz80_ppu_get_sprite_color(gbz80_ppu_t* ppu,uint8_t palette_number, uint8_t color_id);
	uint8_t gbz80_ppu_get_bgp_color(gbz80_ppu_t* ppu, uint8_t color_id);
	uint8_t gbz80_ppu_get_palette_color(gbz80_ppu_t* ppu, uint8_t palette, uint8_t color_id);
	void gbz80_ppu_read_tile_pixels_by_line(gbz80_ppu_t* ppu, uint8_t tile_index, uint8_t tile_line, uint8_t pixels[8], uint8_t adressing_mode);
	void gbz80_ppu_read_sprite(gbz80_ppu_t* ppu, uint8_t sprite_index, gbz80_ppu_sprite_data_t* sprite_data);
	void gbz80_ppu_gather_oam_sprites_by_line(gbz80_ppu_t* ppu, uint8_t ly, gbz80_ppu_sprite_data_t sprites[10], uint8_t* num_sprites);

	//FIFO Things
	void gbz80_ppu_fifo_element_init(gbz80_ppu_fifo_element_t* element, uint8_t palette, uint8_t pixel_value, uint8_t sprite);
	void gbz80_ppu_fifo_init(gbz80_ppu_fifo_t* fifo);
	void gbz80_ppu_fifo_clear(gbz80_ppu_fifo_t* fifo);
	gbz80_ppu_fifo_element_t* gbz80_ppu_fifo_pop(gbz80_ppu_fifo_t* fifo);
	void gbz80_ppu_fifo_push(gbz80_ppu_fifo_t* fifo, gbz80_ppu_fifo_element_t* element);
	void gbz80_ppu_fifo_fetcher_clock(gbz80_ppu_t* ppu, gbz80_ppu_fifo_fetcher_t* fifo_fetcher, uint8_t clock);
	void gbz80_ppu_util_convert_2bpp(uint8_t low, uint8_t high, uint8_t out_pixels[8]);

	void gbz80_ppu_change_state(gbz80_ppu_t* ppu, gbz80_ppu_state_t new_state);
	void gbz80_apply_current_state(gbz80_ppu_t* ppu);

#ifdef __cplusplus
}
#endif