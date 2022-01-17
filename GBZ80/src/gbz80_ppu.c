#include "gbz80_ppu.h"

#include "gbz80.h"

#define NUM_DOTS_TWO (80)
#define NUM_DOTS_THREE (NUM_DOTS_TWO + 172)
#define NUM_DOTS_ZERO (NUM_DOTS_THREE + 204)
#define NUM_DOTS_ONE (NUM_DOTS_ZERO * 144) + 4560


void gbz80_ppu_init(gbz80_ppu_t* ppu, gbz80_t* instance) {
	memset(ppu, 0, sizeof(gbz80_ppu_t));
	ppu->instance = instance;
	gbz80_memory_write8(ppu->instance, 0xFF44, 0x0000);
	gbz80_memory_write8(ppu->instance, 0xFF40, 0x00);

}

void gbz80_ppu_step(gbz80_ppu_t* ppu, size_t num_cycles_passed) {
	uint8_t lcdc = gbz80_memory_read8(ppu->instance, 0xFF40);

	if (common_get8_bit(lcdc, 7) == 1) {
		uint8_t ly = gbz80_memory_read8(ppu->instance, 0xFF44);

		if (ly == 153) {
			ly = 0;
			ppu->num_dots = 0;
		}

		for (size_t num_cycle = 0; num_cycle < num_cycles_passed; num_cycle++) {
			uint32_t time_span = (ppu->num_dots % NUM_DOTS_ZERO) + 1;

			if (ly >= 0 && ly < 143) {
				if (time_span <= NUM_DOTS_TWO) {
					if(time_span == 1)
						gbz80_ppu_update_stat_register(ppu, 2, ly);
				}
				else if (time_span <= NUM_DOTS_THREE) {
					if (time_span == NUM_DOTS_TWO + 1)
						gbz80_ppu_update_stat_register(ppu, 3, ly);

					if (time_span == NUM_DOTS_THREE) {
						if (common_get8_bit(lcdc, 0) == 1) {
							gbz80_ppu_draw_background(ppu, ly);
						}

						if (common_get8_bit(lcdc, 1) == 1) {
							gbz80_ppu_draw_sprites(ppu, ly);
						}
					}


					
				}
				else if (time_span <= NUM_DOTS_ZERO) {
					if (time_span == NUM_DOTS_THREE + 1)
						gbz80_ppu_update_stat_register(ppu, 0, ly);

					if (time_span == NUM_DOTS_ZERO) {
						ly++;
						gbz80_ppu_update_stat_register(ppu, 0, ly);
					}

					
				}
			}
			else {
				if (time_span <= NUM_DOTS_ZERO) {
					if(time_span == 0)
						gbz80_ppu_update_stat_register(ppu, 1, ly);

					if (time_span == NUM_DOTS_ZERO) {
						ly++;
						gbz80_ppu_update_stat_register(ppu, 1, ly);
					}
					
					
				}
			}

			ppu->num_dots++;
		}
	}
	else {
		ppu->num_dots = 0;
		gbz80_ppu_update_stat_register(ppu, 0, 0);
	}
}

void gbz80_ppu_update_stat_register(gbz80_ppu_t* ppu, uint8_t mode, uint8_t ly) {
	gbz80_memory_write8(ppu->instance, 0xFF44, ly);

	uint8_t lyc = gbz80_memory_read8(ppu->instance, 0xFF4A);
	uint8_t stat = gbz80_memory_read8(ppu->instance, 0xFF41);

	common_change8_bit(&stat, 0, mode & 0x1);
	common_change8_bit(&stat, 1, (mode >> 1) & 0x1);
	common_change8_bit(&stat, 2, ly == lyc);

	gbz80_memory_write8(ppu->instance, 0xFF41, stat);
}

void gbz80_ppu_draw_background(gbz80_ppu_t* ppu, uint8_t ly) {
	uint8_t scy = gbz80_memory_read8(ppu->instance, 0xFF42);
	uint8_t scx = gbz80_memory_read8(ppu->instance, 0xFF43);
	uint8_t pixels[8];
	uint8_t tile_y = (scy + ly) / 8;
	uint8_t pixel_line = (scy + ly) % 8;
	uint8_t start_tile_x = scx / 8;

	for (uint8_t tile_index = 0; tile_index < 20; tile_index++) {
		uint8_t current_tile_index = gbz80_ppu_tilemap_read_tile_index_by_coords(ppu, start_tile_x + tile_index, tile_y);
		gbz80_ppu_read_tile_pixels_by_line(ppu, current_tile_index, pixel_line, pixels);

		for (uint8_t pixel_index = 0; pixel_index < 8; pixel_index++) {
			ppu->lcd[ly * 160 + tile_index * 8 + pixel_index] = gbz80_ppu_get_bgp_color(ppu, pixels[pixel_index]);
		}
	}
}

void gbz80_ppu_draw_sprites(gbz80_ppu_t* ppu, uint8_t ly) {
	sprite_data_t sprites[10];
	uint8_t pixels[8];
	uint8_t num_sprites;

	gbz80_ppu_gather_oam_sprites_by_line(ppu, ly, sprites, &num_sprites);
	for (sprite_data_t* sprite = sprites; sprite < sprites + num_sprites; sprite++) {
		uint8_t pixel_line = (ly + 16) - sprite->y;
		gbz80_ppu_read_tile_pixels_by_line(ppu, sprite->tile_index, pixel_line, pixels);

		uint8_t start_pixel = 0;
		uint8_t end_pixel = 8;
		uint8_t tile_index = sprite->x / 8;

		if (sprite->x < 8) {
			start_pixel = 8 - sprite->x;
			end_pixel = 8;
		}
		else if (sprite->x > 160) {
			start_pixel = 0;
			end_pixel = 168 - sprite->x;
		}

		uint8_t palette_number = common_get8_bit(sprite->attributes_flags, 4);
		for (uint8_t pixel_index = start_pixel; pixel_index < end_pixel; pixel_index++) {
			if (pixels[pixel_index] != 0x00) {
				ppu->lcd[ly * 160 + tile_index * 8 + pixel_index] = gbz80_ppu_get_sprite_color(ppu, palette_number, pixels[pixel_index]);
			}
		}

	}
}

void gbz80_ppu_gather_oam_sprites_by_line(gbz80_ppu_t* ppu, uint8_t ly, sprite_data_t sprites[10], uint8_t* num_sprites) {
	uint8_t lcdc = gbz80_memory_read8(ppu->instance, 0xFF40);
	*num_sprites = 0;

	for (uint8_t sprite_index = 0; sprite_index < 40; sprite_index++) {
		sprite_data_t sprite_data = gbz80_ppu_read_sprite(ppu, sprite_index);

		if (sprite_data.x > 0 && sprite_data.x < 168) {
			uint8_t sprite_bottom = sprite_data.y + 8;
			if (common_get8_bit(lcdc, 2) == 1) {
				sprite_bottom += 8;
			}
			uint8_t sprite_top = sprite_data.y;
			uint8_t pixel_check = ly + 16;

			if (pixel_check >= sprite_top && pixel_check <= sprite_bottom) {
				sprites[*num_sprites] = sprite_data;
				(*num_sprites)++;

				if (*num_sprites > 9) {
					break;
				}
			}
		}
	}
}

uint8_t gbz80_ppu_tilemap_read_tile_index_by_coords(gbz80_ppu_t* ppu, uint8_t tile_x, uint8_t tile_y)
{
	tile_x %= 32;
	tile_y %= 32;

	return gbz80_memory_read8(ppu->instance, 0x9800 + tile_y * 32 + tile_x);
}

uint8_t gbz80_ppu_get_sprite_color(gbz80_ppu_t* ppu, uint8_t palette_number, uint8_t color_id)
{
	uint8_t palette = gbz80_memory_read8(ppu->instance, 0xFF48 + palette_number);
	return gbz80_ppu_get_palette_color(ppu, palette, color_id);
}

uint8_t gbz80_ppu_get_bgp_color(gbz80_ppu_t* ppu, uint8_t color_id)
{
	uint8_t bgp = gbz80_memory_read8(ppu->instance, 0xFF47);
	return gbz80_ppu_get_palette_color(ppu, bgp, color_id);
}

uint8_t gbz80_ppu_get_palette_color(gbz80_ppu_t* ppu, uint8_t palette, uint8_t color_id) {
	return (common_get8_bit(palette, color_id * 2 + 1) << 1) | common_get8_bit(palette, color_id * 2);
}

void gbz80_ppu_read_tile_pixels_by_line(gbz80_ppu_t* ppu, uint8_t tile_index, uint8_t tile_line, uint8_t pixels[8]) {
	if (tile_line > 7) {
		pixels = NULL;
		return;
	}

	uint8_t lcdc = gbz80_memory_read8(ppu->instance, 0xFF40);
	uint16_t read_address = 0x0000;
	if (common_get8_bit(lcdc, 4) == 1) {
		read_address = 0x8000 + tile_index * 16 + tile_line * 2;
	} else {
		read_address = 0x9000 + ((int8_t)tile_index) * 16 + tile_line * 2;
	}

	uint8_t low_byte = gbz80_memory_read8(ppu->instance, read_address);
	uint8_t high_byte = gbz80_memory_read8(ppu->instance, read_address + 1);
	for (uint8_t bit_index = 0; bit_index < 8; bit_index++) {
		uint8_t colorID = (common_get8_bit(high_byte, bit_index) << 1) | common_get8_bit(low_byte, bit_index);

		pixels[7 - bit_index] = colorID;
	}
}

sprite_data_t gbz80_ppu_read_sprite(gbz80_ppu_t* ppu, uint8_t sprite_index)
{
	sprite_data_t sprite_data;
	sprite_data.y = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 0);
	sprite_data.x = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 1);
	sprite_data.tile_index = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 2);
	sprite_data.attributes_flags = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 3);

	return sprite_data;
}


