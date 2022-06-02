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
	gbz80_ppu_fifo_init(&ppu->fifo_fetcher.fifo);
	ppu->fifo_fetcher.num_clocks = 0;
	ppu->fifo_fetcher.fifo.stopped = 0;
	ppu->prev_stat_line_or = 0;
	ppu->stat_line = 0;
}

void gbz80_ppu_clock(gbz80_ppu_t* ppu){
	if (common_get8_bit(ppu->lcdc, 7) == 1) {
		uint8_t reset = 0;

		switch (ppu->state) {
			case GBZ80_PPU_STATE_OAM_SCAN: {
				if (ppu->num_dots == 0)
					gbz80_ppu_update_stat_register(ppu, 2);

				if (ppu->num_dots == (NUM_DOTS_START_THREE - 1)) {
					gbz80_ppu_gather_oam_sprites_by_line(ppu, ppu->ly, ppu->oam_sprites, &ppu->num_oam_sprites);
					ppu->scx_dec = ppu->scx & 0x7;

					ppu->mode3_delay = ppu->scx_dec;

					ppu->state = GBZ80_PPU_STATE_DRAWING_PIXELS;
				}

				break;
			}

			case GBZ80_PPU_STATE_DRAWING_PIXELS: {
				if (ppu->num_dots == NUM_DOTS_START_THREE)
					gbz80_ppu_update_stat_register(ppu, 3);

				if (ppu->fifo_fetcher.fifo.size > 8 && ppu->scx_dec > 0 && ppu->lcd_x == 0) {
					gbz80_ppu_fifo_pop(&ppu->fifo_fetcher.fifo);
					ppu->scx_dec--;
				}
				else {
					if (common_get8_bit(ppu->lcdc, 5) && ppu->ly >= ppu->wy) {
						uint8_t last_window_fetch = ppu->fifo_fetcher.window_fetch;

						if (ppu->wx >= 7) {
							ppu->fifo_fetcher.window_fetch = ppu->lcd_x >= (ppu->wx - 7) && ppu->lcd_x <= (ppu->wx - 7 + 160);
						}
						else {
							ppu->fifo_fetcher.window_fetch = ppu->lcd_x <= (ppu->wx - 7 + 160);
						}

						if (last_window_fetch == 0 && ppu->fifo_fetcher.window_fetch == 1) {
							gbz80_ppu_fifo_clear(&ppu->fifo_fetcher.fifo);
							ppu->fifo_fetcher.state = GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_ID;
							ppu->fifo_fetcher.tile_x = ppu->lcd_x / 8;
							ppu->mode3_delay += 6;
						}
					}
					else {
						ppu->fifo_fetcher.window_fetch = 0;
					}

					if (ppu->fifo_fetcher.fifo.size > 8 && ppu->fifo_fetcher.fifo.stopped == 0) {
						


						if (common_get8_bit(ppu->lcdc, 1)) {
							for (gbz80_ppu_sprite_data_t* sprite = ppu->oam_sprites; sprite < ppu->oam_sprites + ppu->num_oam_sprites; sprite++) {
								if ((ppu->lcd_x + 8) == sprite->x) {

									ppu->fifo_fetcher.oam_sprite = sprite;
									ppu->fifo_fetcher.sprite_fetch = 1;
									ppu->fifo_fetcher.fifo.stopped = 1;
									ppu->fifo_fetcher.sprite_fetch_stored_state = ppu->fifo_fetcher.state;
									ppu->fifo_fetcher.state = GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_ID;

									break;
								}
							}
						}

						if (ppu->fifo_fetcher.fifo.size > 8 && ppu->fifo_fetcher.fifo.stopped == 0) {
							gbz80_ppu_fifo_element_t* element = gbz80_ppu_fifo_pop(&ppu->fifo_fetcher.fifo);

							if (!element->sprite) {
								ppu->lcd[ppu->ly * 160 + ppu->lcd_x] = gbz80_ppu_get_bgp_color(ppu, element->pixel_value);
							}
							else {
								ppu->lcd[ppu->ly * 160 + ppu->lcd_x] = gbz80_ppu_get_sprite_color(ppu, element->palette, element->pixel_value);
							}

							ppu->lcd_x++;
						}
					}

					gbz80_ppu_fifo_fetcher_clock(ppu, &ppu->fifo_fetcher, ppu->num_dots);
				}

				if (ppu->num_dots == (NUM_DOTS_START_ZERO - 1 + ppu->mode3_delay)) {
					ppu->state = GBZ80_PPU_STATE_HMODE;
				}
				break;
			}

			case GBZ80_PPU_STATE_HMODE: {
				if (ppu->num_dots == (NUM_DOTS_START_ZERO + ppu->mode3_delay)) {
					gbz80_ppu_update_stat_register(ppu, 0);

					ppu->lcd_x = 0;
					ppu->fifo_fetcher.tile_x = 0;
					ppu->fifo_fetcher.num_clocks = 0;
					gbz80_ppu_fifo_clear(&ppu->fifo_fetcher.fifo);
					ppu->fifo_fetcher.fifo.stopped = 0;
					ppu->fifo_fetcher.state = GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_ID;
				}
				else if (ppu->num_dots == (NUM_DOTS_PER_LINE - 1)) {
					if (ppu->ly >= 0 && ppu->ly < 143) {
						ppu->state = GBZ80_PPU_STATE_OAM_SCAN;
					}
					else {
						ppu->state = GBZ80_PPU_STATE_VBLANK;
					}

					ppu->ly++;
					reset = 1;
				}
				break;
			}

			case GBZ80_PPU_STATE_VBLANK: {
				if (ppu->num_dots == 0) {
					gbz80_ppu_update_stat_register(ppu, 1);

					if (ppu->ly == 144) {
						gbz80_cpu_request_interrupt(&ppu->instance->cpu, GBZ80_INTERRUPT_VBLANK);
					}
				}
				else if (ppu->num_dots == (NUM_DOTS_PER_LINE - 1)) {
					ppu->ly++;

					if (ppu->ly == 153) {
						ppu->ly = 0;
						ppu->state = GBZ80_PPU_STATE_OAM_SCAN;
					}

					reset = 1;
				}
				break;
			}
		}

		ppu->num_dots++;
		if (reset) {
			ppu->num_dots = 0;
		}
	}
	else {
		ppu->num_dots = 0;
		gbz80_ppu_update_stat_register(ppu, 0);
	}
}

void gbz80_ppu_update_stat_register(gbz80_ppu_t* ppu, uint8_t mode) {
	common_change8_bit_range(&ppu->lcd_status, 0, 1, mode);
	common_change8_bit(&ppu->lcd_status, 2, ppu->ly == ppu->lyc);

	for (uint8_t i = 0; i < 3; i++) {
		if (i == mode && common_get8_bit(ppu->lcd_status, 3 + i)) {
			common_set8_bit(&ppu->stat_line, i);
		} else {
			common_reset8_bit(&ppu->stat_line, i);
		}
	}

	if (ppu->ly == ppu->lyc && common_get8_bit(ppu->lcd_status, 6)) {
		common_set8_bit(&ppu->stat_line, 3);
	} else {
		common_reset8_bit(&ppu->stat_line, 3);
	}

	uint8_t current_stat_line_or = 0;
	for (uint8_t i = 0; i < 4; i++) {
		current_stat_line_or |= common_get8_bit(ppu->stat_line, i);
	}

	if (current_stat_line_or == 1 && ppu->prev_stat_line_or == 0) {
		gbz80_cpu_request_interrupt(&ppu->instance->cpu, GBZ80_INTERRUPT_LCDSTAT);
	}

	ppu->prev_stat_line_or = current_stat_line_or;
}

void gbz80_ppu_memory_read(gbz80_ppu_t* ppu, uint16_t address, uint8_t* val) {
	switch (address) {
		case 0xFF40:
			*val = ppu->lcdc;
			break;

		case 0xFF41:
			*val = ppu->lcd_status;
			break;

		case 0xFF42:
			*val = ppu->scy;
			break;

		case 0xFF43:
			*val = ppu->scx;
			break;

		case 0xFF44:
			*val = ppu->ly;
			break;

		case 0xFF45:
			*val = ppu->lyc;
			break;

		case 0xFF47:
			*val = ppu->bgp;
			break;

		case 0xFF48:
			*val = ppu->obp[0];
			break;

		case 0xFF49:
			*val = ppu->obp[1];
			break;

		case 0xFF4A:
			*val = ppu->wy;
			break;

		case 0xFF4B:
			*val = ppu->wx;
			break;
	}
}

uint8_t gbz80_ppu_memory_write(gbz80_ppu_t* ppu, uint16_t address, uint8_t current_value, uint8_t* val)
{
	switch (address) {
		case 0xFF40:
			ppu->lcdc = *val;
			break;

		case 0xFF41:
			ppu->lcd_status |= (*val & 0x78);
			break;

		case 0xFF42:
			ppu->scy = *val;
			break;

		case 0xFF43:
			ppu->scx = *val;
			break;

		case 0xFF44:
			//Ignore write
			*val = ppu->ly;
			break;

		case 0xFF45:
			ppu->lyc = *val;
			break;

		case 0xFF47:
			ppu->bgp = *val;
			break;

		case 0xFF48:
			ppu->obp[0] = *val;
			break;

		case 0xFF49:
			ppu->obp[1] = *val;
			break;

		case 0xFF4A:
			ppu->wy = *val;
			break;

		case 0xFF4B:
			ppu->wx = *val;
			break;
	}

	return 1;
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
	*num_sprites = 0;

	for (uint8_t sprite_index = 0; sprite_index < 40; sprite_index++) {
		gbz80_ppu_sprite_data_t* sprite_data = sprites + (*num_sprites);
		gbz80_ppu_read_sprite(ppu, sprite_index, sprite_data);

		uint8_t sprite_top = sprite_data->y;
		uint8_t sprite_bottom = sprite_data->y + 8;
		if (common_get8_bit(ppu->lcdc, 2) == 1) {
			sprite_bottom += 8;
		}
		uint8_t pixel_check = ly + 16;

		if (pixel_check >= sprite_top && pixel_check < sprite_bottom) {
			(*num_sprites)++;

			if (*num_sprites > 9) {
				break;
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

	return gbz80_memory_read8(ppu->instance, base_address + tile_y * 32 + tile_x);
}

uint8_t gbz80_ppu_get_sprite_color(gbz80_ppu_t* ppu, uint8_t palette_number, uint8_t color_id) {
	assert(palette_number < 2 && "Selected sprite palette does not exists");
	return gbz80_ppu_get_palette_color(ppu, ppu->obp[palette_number], color_id);
}

uint8_t gbz80_ppu_get_bgp_color(gbz80_ppu_t* ppu, uint8_t color_id) {
	return gbz80_ppu_get_palette_color(ppu, ppu->bgp, color_id);
}

uint8_t gbz80_ppu_get_palette_color(gbz80_ppu_t* ppu, uint8_t palette, uint8_t color_id) {
	return (common_get8_bit(palette, color_id * 2 + 1) << 1) | common_get8_bit(palette, color_id * 2);
}

void gbz80_ppu_read_tile_pixels_by_line(gbz80_ppu_t* ppu, uint8_t tile_index, uint8_t tile_line, uint8_t pixels[8], uint8_t adressing_mode) {
	if (tile_line > 7) {
		pixels = NULL;
		return;
	}

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
	sprite_data->y = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 0);
	sprite_data->x = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 1);
	sprite_data->tile_index = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 2);
	sprite_data->attributes_flags = gbz80_memory_read8(ppu->instance, 0xFE00 + sprite_index * 4 + 3);
}

//FIFO
void gbz80_ppu_fifo_element_init(gbz80_ppu_fifo_element_t* element, uint8_t palette, uint8_t pixel_value, uint8_t sprite) {
	element->palette = palette;
	element->pixel_value = pixel_value;
	element->sprite = sprite;
}

void gbz80_ppu_fifo_init(gbz80_ppu_fifo_t* fifo) {
	fifo->size = 0;
	fifo->write_index = 0;
	fifo->read_index = 0;
	memset(fifo->data, 0, sizeof(gbz80_ppu_fifo_element_t) * 16);
}

void gbz80_ppu_fifo_clear(gbz80_ppu_fifo_t* fifo) {
	fifo->size = 0;
	fifo->read_index = 0;
	fifo->write_index = 0;
#ifdef _DEBUG
	memset(fifo->data, 0, sizeof(gbz80_ppu_fifo_element_t) * 16);
#endif
}

gbz80_ppu_fifo_element_t* gbz80_ppu_fifo_pop(gbz80_ppu_fifo_t* fifo) {
	assert(fifo->size != 0 && "Failed to pop from PPU Fifo is empty");
	gbz80_ppu_fifo_element_t* element = &fifo->data[fifo->read_index++];
	fifo->size--;
	fifo->read_index %= 16;
	return element;
}

void gbz80_ppu_fifo_push(gbz80_ppu_fifo_t* fifo, gbz80_ppu_fifo_element_t* element) {
	assert(fifo->size < 16 && "Failed to push from PPU Fifo is full");
	memcpy(&fifo->data[fifo->write_index++], element, sizeof(gbz80_ppu_fifo_element_t));
	fifo->size++;
	fifo->write_index %= 16;
}

void gbz80_ppu_fifo_fetcher_clock(gbz80_ppu_t* ppu, gbz80_ppu_fifo_fetcher_t* fifo_fetcher, uint8_t clock) {

	if (ppu->lcd_x < 160) {
		if (clock % 2) {
			uint8_t reset = 0;

			switch (fifo_fetcher->state) {
				case GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_ID: {
					uint8_t bg_enable = common_get8_bit(ppu->lcdc, 0);
					uint8_t sprite_mode = common_get8_bit(ppu->lcdc, 2);
					uint8_t bg_map_index = common_get8_bit(ppu->lcdc, 3);					
					uint8_t win_map_index = common_get8_bit(ppu->lcdc, 6);

					if (fifo_fetcher->sprite_fetch) {
						uint8_t offset_y = ppu->ly - fifo_fetcher->oam_sprite->y + 16;

						if (sprite_mode) {
							fifo_fetcher->sprite_tile_index = fifo_fetcher->oam_sprite->tile_index & 0xFE;
							if (offset_y > 7) {
								offset_y -= 8;
								fifo_fetcher->sprite_tile_index = fifo_fetcher->oam_sprite->tile_index | 0x01;
							}
						} else {
							fifo_fetcher->sprite_tile_index = fifo_fetcher->oam_sprite->tile_index;
						}

						if (common_get8_bit(fifo_fetcher->oam_sprite->attributes_flags, 6)) {
							fifo_fetcher->sprite_fetch_y = 7 - offset_y;
						} else {
							fifo_fetcher->sprite_fetch_y = offset_y;
						}

						if (!fifo_fetcher->window_fetch) {
							ppu->mode3_delay += 11 - min(5, (fifo_fetcher->oam_sprite->x + ppu->scx) % 8);
						} else {
							ppu->mode3_delay += 11 - min(5, (fifo_fetcher->oam_sprite->x + (255 - ppu->wx)) % 8);
						}
					} else {
						if (fifo_fetcher->window_fetch) {
							if (ppu->wx >= 7) {
								fifo_fetcher->fetch_x = (fifo_fetcher->tile_x - ((ppu->wx - 7) / 8)) & 0x1F;
							}
							else {
								fifo_fetcher->fetch_x = fifo_fetcher->tile_x == 0 ? 0 : fifo_fetcher->tile_x - 1;
							}

							fifo_fetcher->fetch_y = (ppu->ly - ppu->wy) & 0xFF;

							uint8_t tile_x = fifo_fetcher->fetch_x;
							uint8_t tile_y = fifo_fetcher->fetch_y / 8;

							fifo_fetcher->tile_index = gbz80_ppu_tilemap_read_tile_index_by_coords(ppu, tile_x, tile_y, (gbz80_ppu_tilemap_type_t)win_map_index);
						}
						else {
							if (bg_enable) {
								fifo_fetcher->fetch_x = ((ppu->scx / 8) + fifo_fetcher->tile_x) & 0x1F;
								fifo_fetcher->fetch_y = (ppu->scy + ppu->ly) & 0xFF;

								uint8_t tile_x = fifo_fetcher->fetch_x;
								uint8_t tile_y = fifo_fetcher->fetch_y / 8;

								fifo_fetcher->tile_index = gbz80_ppu_tilemap_read_tile_index_by_coords(ppu, tile_x, tile_y, (gbz80_ppu_tilemap_type_t)bg_map_index);
							}
						}

						fifo_fetcher->tile_x++;
					}

					fifo_fetcher->state = GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_LOW;
					break;
				}

				case GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_LOW: {
					uint16_t read_address = 0x0000;
					uint8_t adressing_mode = common_get8_bit(ppu->lcdc, 4);
					if (fifo_fetcher->sprite_fetch == 1) {
						adressing_mode = 1;
					}

					uint8_t tile_index = fifo_fetcher->sprite_fetch ? fifo_fetcher->sprite_tile_index : fifo_fetcher->tile_index;
					uint8_t fetch_y = fifo_fetcher->sprite_fetch ? fifo_fetcher->sprite_fetch_y : fifo_fetcher->fetch_y;

					if (adressing_mode == 1) {
						read_address = 0x8000 + tile_index * 16 + (fetch_y % 8) * 2;
					} else {
						read_address = 0x9000 + ((int8_t)tile_index) * 16 + (fetch_y % 8) * 2;
					}

					uint8_t tile_low = gbz80_memory_read8(ppu->instance, read_address);
					if (fifo_fetcher->sprite_fetch == 1) {
						fifo_fetcher->sprite_tile_low = tile_low;
					} else {
						fifo_fetcher->tile_low = tile_low;
					}

					fifo_fetcher->state = GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_HIGH;
					break;
				}

				case GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_HIGH: {
					uint16_t read_address = 0x0000;
					uint8_t adressing_mode = common_get8_bit(ppu->lcdc, 4);
					if (fifo_fetcher->sprite_fetch == 1) {
						adressing_mode = 1;
					}

					uint8_t tile_index = fifo_fetcher->sprite_fetch ? fifo_fetcher->sprite_tile_index : fifo_fetcher->tile_index;
					uint8_t fetch_y = fifo_fetcher->sprite_fetch ? fifo_fetcher->sprite_fetch_y : fifo_fetcher->fetch_y;

					if (adressing_mode == 1) {
						read_address = 0x8000 + tile_index * 16 + (fetch_y % 8) * 2;
					} else {
						read_address = 0x9000 + ((int8_t)tile_index) * 16 + (fetch_y % 8) * 2;
					}

					uint8_t tile_high = gbz80_memory_read8(ppu->instance, read_address + 1);
					if (fifo_fetcher->sprite_fetch == 1) {
						fifo_fetcher->sprite_tile_high = tile_high;
					} else {
						fifo_fetcher->tile_high = tile_high;
					}

					if (fifo_fetcher->fifo.size > 8 || fifo_fetcher->sprite_fetch) {
						fifo_fetcher->state = GBZ80_PPU_FIFO_FETCHER_STATE_PUSH;
						break;
					}
				}
				
				case GBZ80_PPU_FIFO_FETCHER_STATE_PUSH: {
					uint8_t pixels[8];
					gbz80_ppu_fifo_element_t element;

					if (fifo_fetcher->sprite_fetch) {
						//Mix
						gbz80_ppu_util_convert_2bpp(fifo_fetcher->sprite_tile_low, fifo_fetcher->sprite_tile_high, pixels);

						for (uint8_t i = 0; i < 8; i++) {
							uint8_t pixel_index = i;
							if (common_get8_bit(fifo_fetcher->oam_sprite->attributes_flags, 5)) {
								pixel_index = 7 - i;
							}

							uint8_t index = (ppu->fifo_fetcher.fifo.read_index + i) % 16;
							gbz80_ppu_fifo_element_t* element = &fifo_fetcher->fifo.data[index];
							uint8_t pixel_val = pixels[pixel_index];

							//If we have a pixel that is already in the fifo and it is a sprite we can't get priority over it because smaller X = higher priority in Non-CGB
							//Pixel with value 0 are treated as transparent so they are skipped
							//If the sprite has OBJ-to-BG Priority (Bit 7 of OAM Attributes) set, BG Values (1-3) goes over the sprite
							if (!element->sprite && pixel_val != 0 && (!common_get8_bit(fifo_fetcher->oam_sprite->attributes_flags, 7) || element->pixel_value == 0)) {
								element->sprite = 1;
								element->pixel_value = pixel_val;
								element->palette = common_get8_bit(fifo_fetcher->oam_sprite->attributes_flags, 4);
							}
						}

						ppu->fifo_fetcher.state = ppu->fifo_fetcher.sprite_fetch_stored_state;
						fifo_fetcher->sprite_fetch = 0;
						fifo_fetcher->oam_sprite->x = 0;
						fifo_fetcher->oam_sprite = NULL;
						fifo_fetcher->fifo.stopped = 0;
					} else {
						gbz80_ppu_util_convert_2bpp(fifo_fetcher->tile_low, fifo_fetcher->tile_high, pixels);

						for (uint8_t i = 0; i < 8; i++) {
							gbz80_ppu_fifo_element_init(&element, 0, pixels[i], 0);
							gbz80_ppu_fifo_push(&fifo_fetcher->fifo, &element);
						}
						fifo_fetcher->state = GBZ80_PPU_FIFO_FETCHER_STATE_READ_TILE_ID;
					}

					reset = 1;
					break;
				}
			}

			fifo_fetcher->num_clocks++;
			if (reset) {
				fifo_fetcher->num_clocks = 0;
			}
		}
	}
}

void gbz80_ppu_util_convert_2bpp(uint8_t low, uint8_t high, uint8_t out_pixels[8]) {
	for (uint8_t bit_index = 0; bit_index < 8; bit_index++) {
		uint8_t colorID = (common_get8_bit(high, bit_index) << 1) | common_get8_bit(low, bit_index);
		out_pixels[7 - bit_index] = colorID;
	}
}