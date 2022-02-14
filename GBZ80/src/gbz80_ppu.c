#include "gbz80_ppu.h"

#include "gbz80.h"

#define NUM_DOTS_PER_LINE 456
#define NUM_DOTS_START_TWO 0
#define NUM_DOTS_START_THREE 80
#define NUM_DOTS_START_ZERO 252

void gbz80_ppu_init(gbz80_ppu_t* ppu, gbz80_t* instance) {
	memset(ppu, 0, sizeof(gbz80_ppu_t));
	ppu->instance = instance;
	gbz80_memory_write_internal(ppu->instance, 0xFF44, 0x0000);
	gbz80_memory_write_internal(ppu->instance, 0xFF40, 0x00);
}

void gbz80_ppu_clock(gbz80_ppu_t* ppu){
	uint8_t lcdc = gbz80_memory_read_internal(ppu->instance, 0xFF40);

	if (common_get8_bit(lcdc, 7) == 1) {
		uint8_t ly = gbz80_memory_read_internal(ppu->instance, 0xFF44);

		if (ly >= 0 && ly < 143) {
			if (ppu->num_dots == NUM_DOTS_START_TWO) {
				gbz80_ppu_update_stat_register(ppu, 2, ly);
			}
			else if (ppu->num_dots == NUM_DOTS_START_THREE) {
				if (common_get8_bit(lcdc, 0) == 1) {
					gbz80_ppu_draw_background(ppu, ly, lcdc);
				}

				if (common_get8_bit(lcdc, 5) == 1) {
					gbz80_ppu_draw_window(ppu, ly, lcdc);
				}

				if (common_get8_bit(lcdc, 1) == 1) {
					gbz80_ppu_draw_sprites(ppu, ly, lcdc);
				}

				

				gbz80_ppu_update_stat_register(ppu, 3, ly);
			}
			else if (ppu->num_dots == NUM_DOTS_START_ZERO) {
				gbz80_ppu_update_stat_register(ppu, 0, ly);
			}
		}

		ppu->num_dots++;

		if (ppu->num_dots == NUM_DOTS_PER_LINE) {
			ppu->num_dots = 0;
			if (ly < 143) {
				gbz80_ppu_update_stat_register(ppu, 2, ly + 1);
			} else {
				if (ly == 153) {
					gbz80_ppu_update_stat_register(ppu, 2, 0);
				} else {
					gbz80_ppu_update_stat_register(ppu, 1, ly + 1);
					if (ly == 143) {
						gbz80_cpu_request_interrupt(&ppu->instance->cpu, GBZ80_INTERRUPT_VBLANK);
					}
				}
			}
		}
	}
	else {
		ppu->num_dots = 0;
		gbz80_ppu_update_stat_register(ppu, 0, 0);
	}
}

void gbz80_ppu_update_stat_register(gbz80_ppu_t* ppu, uint8_t mode, uint8_t ly) {
	gbz80_memory_write_internal(ppu->instance, 0xFF44, ly);

	uint8_t lyc = gbz80_memory_read_internal(ppu->instance, 0xFF4A);
	uint8_t stat = gbz80_memory_read_internal(ppu->instance, 0xFF41);

	common_change8_bit(&stat, 0, mode & 0x1);
	common_change8_bit(&stat, 1, (mode >> 1) & 0x1);
	common_change8_bit(&stat, 2, ly == lyc);

	gbz80_memory_write_internal(ppu->instance, 0xFF41, stat);
}

void gbz80_ppu_draw_background(gbz80_ppu_t* ppu, uint8_t ly, uint8_t lcdc) {
	uint8_t scy = gbz80_memory_read_internal(ppu->instance, 0xFF42);
	uint8_t scx = gbz80_memory_read_internal(ppu->instance, 0xFF43);
	uint8_t pixels[8];
	uint8_t tile_y = (scy + ly) / 8;
	uint8_t pixel_line = (scy + ly) % 8;
	uint8_t start_tile_x = scx / 8;
	uint8_t adressing_mode = common_get8_bit(lcdc, 4);
	uint8_t map_index = common_get8_bit(lcdc, 3);

	for (uint8_t tile_index = 0; tile_index < 20; tile_index++) {
		uint8_t current_tile_index = gbz80_ppu_tilemap_read_tile_index_by_coords(ppu, start_tile_x + tile_index, tile_y, (gbz80_ppu_tilemap_type_t)map_index);
		gbz80_ppu_read_tile_pixels_by_line(ppu, current_tile_index, pixel_line, pixels, adressing_mode);

		for (uint8_t pixel_index = 0; pixel_index < 8; pixel_index++) {
			ppu->lcd[ly * 160 + tile_index * 8 + pixel_index] = gbz80_ppu_get_bgp_color(ppu, pixels[pixel_index]);
		}
	}
}

void gbz80_ppu_draw_window(gbz80_ppu_t* ppu, uint8_t ly, uint8_t lcdc) {
	uint8_t pixels[8];

	uint8_t wy = gbz80_memory_read_internal(ppu->instance, 0xFF4A);

	if (ly >= wy) {
		uint8_t wx = gbz80_memory_read_internal(ppu->instance, 0xFF4B);
		
		uint8_t start_tile_index = wx == 0 ? 1 : (wx < 7 ? 0 : (wx - 7 / 8));
		uint8_t num_shown_tiles = 20 - start_tile_index;
		uint8_t tile_y = (wy - ly) / 8;
		uint8_t adressing_mode = common_get8_bit(lcdc, 4);
		uint8_t map_index = common_get8_bit(lcdc, 6);
		uint8_t pixel_line = (wy - ly) % 8;


		for (uint8_t tile_x = start_tile_index; tile_x < num_shown_tiles; tile_x++) {
			uint8_t current_tile_index = gbz80_ppu_tilemap_read_tile_index_by_coords(ppu, tile_x, tile_y, (gbz80_ppu_tilemap_type_t)map_index);
			gbz80_ppu_read_tile_pixels_by_line(ppu, current_tile_index, pixel_line, pixels, adressing_mode);

			uint8_t start_pixel_x = 0;
			uint8_t end_pixel_x = 8;

			if (tile_x == start_tile_index && wx < 7 && wx > 0) {
				start_pixel_x = 7 - wx;
			}

			for (uint8_t pixel_index = start_pixel_x; pixel_index < end_pixel_x; pixel_index++) {
				ppu->lcd[ly * 160 + tile_x * 8 + (pixel_index - start_pixel_x)] = gbz80_ppu_get_bgp_color(ppu, pixels[pixel_index]);
			}
		}
	}
}

void gbz80_ppu_draw_sprites(gbz80_ppu_t* ppu, uint8_t ly, uint8_t lcdc) {
	gbz80_ppu_sprite_data_t sprites[10];
	uint8_t pixels[8];
	uint8_t num_sprites;
	uint8_t sprite_mode = common_get8_bit(lcdc, 2);

	gbz80_ppu_gather_oam_sprites_by_line(ppu, ly, sprites, &num_sprites);
	for (gbz80_ppu_sprite_data_t* sprite = sprites; sprite < sprites + num_sprites; sprite++) {
		uint8_t pixel_line = (ly + 16) - sprite->y;

		if (sprite_mode == 0) {
			gbz80_ppu_read_tile_pixels_by_line(ppu, sprite->tile_index, pixel_line, pixels, 1);
		}
		else {
			if (pixel_line < 8) {
				gbz80_ppu_read_tile_pixels_by_line(ppu, sprite->tile_index & 0xFE, pixel_line, pixels, 1);
			}
			else {
				gbz80_ppu_read_tile_pixels_by_line(ppu, sprite->tile_index & 0x01, pixel_line, pixels, 1);
			}
		}

		if (common_get8_bit(sprite->attributes_flags, 5)) {
			uint8_t aux[8];

			for (uint8_t i = 0; i < 8; i++) {
				aux[7 - i] = pixels[i];
			}

			for (uint8_t i = 0; i < 8; i++) {
				pixels[i] = aux[i];
			}
		}

		uint8_t start_pixel = 0;
		uint8_t end_pixel = 8;
		uint8_t tile_index = sprite->x / 8 - 1;

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
			uint8_t pixel_color = gbz80_ppu_get_sprite_color(ppu, palette_number, pixels[pixel_index]);;
			if (pixel_color != 0x00) {
				ppu->lcd[ly * 160 + tile_index * 8 + pixel_index] = pixel_color;
			}
		}

	}
}

void gbz80_ppu_gather_oam_sprites_by_line(gbz80_ppu_t* ppu, uint8_t ly, gbz80_ppu_sprite_data_t sprites[10], uint8_t* num_sprites) {
	uint8_t lcdc = gbz80_memory_read_internal(ppu->instance, 0xFF40);
	*num_sprites = 0;

	for (uint8_t sprite_index = 0; sprite_index < 40; sprite_index++) {
		gbz80_ppu_sprite_data_t* sprite_data = sprites + (*num_sprites);

		gbz80_ppu_read_sprite(ppu, sprite_index, sprite_data);

		if (sprite_data->x > 0 && sprite_data->x < 168) {
			uint8_t sprite_bottom = sprite_data->y + 8;
			if (common_get8_bit(lcdc, 2) == 1) {
				sprite_bottom += 8;
			}
			uint8_t sprite_top = sprite_data->y;
			uint8_t pixel_check = ly + 16;

			if (pixel_check >= sprite_top && pixel_check <= sprite_bottom) {
				(*num_sprites)++;

				if (*num_sprites > 9) {
					break;
				}
			}
		}
	}
}

uint8_t gbz80_ppu_tilemap_read_tile_index_by_coords(gbz80_ppu_t* ppu, uint8_t tile_x, uint8_t tile_y, gbz80_ppu_tilemap_type_t map_type){
	tile_x %= 32;
	tile_y %= 32;

	uint16_t base_address = 0;
	switch (map_type) {
		case GBZ80_PPU_TILEMAP_0:
			base_address = 0x9800;
			break;

		case GBZ80_PPU_TILEMAP_1:
			base_address = 0x9C00;
			break;
	}

	return gbz80_memory_read_internal(ppu->instance, base_address + tile_y * 32 + tile_x);
}

uint8_t gbz80_ppu_get_sprite_color(gbz80_ppu_t* ppu, uint8_t palette_number, uint8_t color_id)
{
	uint8_t palette = gbz80_memory_read_internal(ppu->instance, 0xFF48 + palette_number);
	return gbz80_ppu_get_palette_color(ppu, palette, color_id);
}

uint8_t gbz80_ppu_get_bgp_color(gbz80_ppu_t* ppu, uint8_t color_id)
{
	uint8_t bgp = gbz80_memory_read_internal(ppu->instance, 0xFF47);
	return gbz80_ppu_get_palette_color(ppu, bgp, color_id);
}

uint8_t gbz80_ppu_get_palette_color(gbz80_ppu_t* ppu, uint8_t palette, uint8_t color_id) {
	return (common_get8_bit(palette, color_id * 2 + 1) << 1) | common_get8_bit(palette, color_id * 2);
}

void gbz80_ppu_read_tile_pixels_by_line(gbz80_ppu_t* ppu, uint8_t tile_index, uint8_t tile_line, uint8_t pixels[8], uint8_t adressing_mode) {
	if (tile_line > 7) {
		pixels = NULL;
		return;
	}

	uint8_t lcdc = gbz80_memory_read_internal(ppu->instance, 0xFF40);
	uint16_t read_address = 0x0000;
	if (adressing_mode == 1) {
		read_address = 0x8000 + tile_index * 16 + tile_line * 2;
	} else {
		read_address = 0x9000 + ((int8_t)tile_index) * 16 + tile_line * 2;
	}

	uint8_t low_byte = gbz80_memory_read_internal(ppu->instance, read_address);
	uint8_t high_byte = gbz80_memory_read_internal(ppu->instance, read_address + 1);
	for (uint8_t bit_index = 0; bit_index < 8; bit_index++) {
		uint8_t colorID = (common_get8_bit(high_byte, bit_index) << 1) | common_get8_bit(low_byte, bit_index);

		pixels[7 - bit_index] = colorID;
	}
}

void gbz80_ppu_read_sprite(gbz80_ppu_t* ppu, uint8_t sprite_index, gbz80_ppu_sprite_data_t* sprite_data) {
	sprite_data->y = gbz80_memory_read_internal(ppu->instance, 0xFE00 + sprite_index * 4 + 0);
	sprite_data->x = gbz80_memory_read_internal(ppu->instance, 0xFE00 + sprite_index * 4 + 1);
	sprite_data->tile_index = gbz80_memory_read_internal(ppu->instance, 0xFE00 + sprite_index * 4 + 2);
	sprite_data->attributes_flags = gbz80_memory_read_internal(ppu->instance, 0xFE00 + sprite_index * 4 + 3);
}



//FIFO
void gbz80_ppu_fifo_element_init(gbz80_ppu_fifo_element_t* element, uint8_t palette, uint8_t pixel_value, uint8_t sprite) {
	element->palette = palette;
	element->pixel_value = pixel_value;
	element->sprite = sprite;
}

void gbz80_ppu_fifo_init(gbz80_ppu_fifo_t* fifo) {
	fifo->size = 0;
	memset(fifo->data, 0, sizeof(gbz80_ppu_fifo_element_t) * 16);
}

void gbz80_ppu_fifo_clear(gbz80_ppu_fifo_t* fifo) {
	fifo->size = 0;
#ifdef _DEBUG
	memset(fifo->data, 0, sizeof(gbz80_ppu_fifo_element_t) * 16);
#endif
}

gbz80_ppu_fifo_element_t* gbz80_ppu_fifo_pop(gbz80_ppu_fifo_t* fifo) {
	assert(fifo->size != 0 && "Failed to pop from PPU Fifo is empty");
	gbz80_ppu_fifo_element_t* element = &fifo->data[fifo->size - 1];
	fifo->size--;
	return element;
}

void gbz80_ppu_fifo_push(gbz80_ppu_fifo_t* fifo, gbz80_ppu_fifo_element_t* element) {
	assert(fifo->size < 16 && "Failed to push from PPU Fifo is full");
	memcpy(&fifo->data[fifo->size], element, sizeof(gbz80_ppu_fifo_element_t));
	fifo->size++;
}

void gbz80_ppu_fifo_fetcher_clock(gbz80_ppu_t* ppu, gbz80_ppu_fifo_fetcher_t* fifo_fetcher, uint8_t ly, uint8_t lcdc) {
	uint8_t scy = gbz80_memory_read_internal(ppu->instance, 0xFF42);
	uint8_t scx = gbz80_memory_read_internal(ppu->instance, 0xFF43);
	uint8_t wy = gbz80_memory_read_internal(ppu->instance, 0xFF4A);
	uint8_t wx = gbz80_memory_read_internal(ppu->instance, 0xFF4B);
	uint8_t bg_enable = common_get8_bit(lcdc, 0);
	uint8_t sprite_enable = common_get8_bit(lcdc, 1);
	uint8_t sprite_mode = common_get8_bit(lcdc, 2);
	uint8_t bg_map_index = common_get8_bit(lcdc, 3);
	uint8_t adressing_mode = common_get8_bit(lcdc, 4);
	uint8_t win_enable = common_get8_bit(lcdc, 5);
	uint8_t win_map_index = common_get8_bit(lcdc, 6);
	/*
	uint16_t read_address = 0x0000;
	if (adressing_mode == 1) {
		read_address = 0x8000 + tile_index * 16 + tile_line * 2;
	}
	else {
		read_address = 0x9000 + ((int8_t)tile_index) * 16 + tile_line * 2;
	}
	*/

	switch (fifo_fetcher->num_clocks) {
	case 0: {
		if (sprite_enable) {

		} else if (win_enable && ly >= wy && ppu->lcd_x >= wx && ppu->lcd_x <= (wx + 152)) {
			if (ppu->lcd_x == wx) {
				gbz80_ppu_fifo_clear(&fifo_fetcher->fifo);
			}
			fifo_fetcher->tile_index = gbz80_ppu_tilemap_read_tile_index_by_coords(ppu, ((wx - 7) - ppu->lcd_x) / 8 , (wy - ly) / 8, (gbz80_ppu_tilemap_type_t)win_map_index);
		} else if (bg_enable) {
			fifo_fetcher->tile_index = gbz80_ppu_tilemap_read_tile_index_by_coords(ppu, ppu->lcd_x / 8, ly / 8, (gbz80_ppu_tilemap_type_t)bg_map_index);
		}
		break;
	}
		case 2:
			//uint8_t low_byte = gbz80_memory_read_internal(ppu->instance, read_address);
			//uint8_t high_byte = gbz80_memory_read_internal(ppu->instance, read_address + 1);
			break;

		case 4:
			break;

		case 6:
			break;
	}

	if (fifo_fetcher->fifo.size > 8) {
		gbz80_ppu_fifo_element_t* element = gbz80_ppu_fifo_pop(&fifo_fetcher->fifo);

		if (!element->sprite) {
			ppu->lcd[ly * 160 + ppu->lcd_x] = gbz80_ppu_get_bgp_color(ppu, element->pixel_value);
		}
		else {
			ppu->lcd[ly * 160 + ppu->lcd_x] = gbz80_ppu_get_sprite_color(ppu, element->palette, element->pixel_value);
		}

		ppu->lcd_x++;
	}
	

	fifo_fetcher->num_clocks++;
	fifo_fetcher->num_clocks %= 8;
}

