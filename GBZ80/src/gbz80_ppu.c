#include "gbz80_ppu.h"

#include "gbz80.h"

#define NUM_DOTS_TWO (80)
#define NUM_DOTS_THREE (NUM_DOTS_TWO + 172)
#define NUM_DOTS_ZERO (NUM_DOTS_THREE + 204)
#define NUM_DOTS_ONE (NUM_DOTS_ZERO * 144) + 4560


void gbz80_ppu_init(gbz80_ppu_t* ppu, gbz80_t* instance) {
	memset(ppu, 0, sizeof(gbz80_ppu_t));
	ppu->instance = instance;
	gbz80_cpu_memory_write8(&ppu->instance->cpu, 0xFF44, 0x0000);
	gbz80_cpu_memory_write8(&ppu->instance->cpu, 0xFF40, 0x00);

}

void gbz80_ppu_step(gbz80_ppu_t* ppu, size_t num_cycles_passed) {
	uint8_t lcdc = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF40);

	if (common_get8_bit(lcdc, 7) == 1) {
		uint8_t ly = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF44);

		if (ly == 153) {
			ly = 0;
			ppu->num_dots = 0;
		}

		for (size_t num_cycle = 0; num_cycle < num_cycles_passed; num_cycle++) {
			uint32_t time_span = (ppu->num_dots % NUM_DOTS_ZERO) + 1;

			if (ly >= 0 && ly < 143) {
				if (time_span <= NUM_DOTS_TWO) {
					gbz80_ppu_update_stat_register(ppu, 2, ly);
				}
				else if (time_span <= NUM_DOTS_THREE) {
					if (time_span == NUM_DOTS_THREE) {
						if (common_get8_bit(lcdc, 0) == 1) {
							gbz80_ppu_draw_background(ppu, ly);
						}
					}

					gbz80_ppu_update_stat_register(ppu, 3, ly);
				}
				else if (time_span <= NUM_DOTS_ZERO) {
					if (time_span == NUM_DOTS_ZERO) {
						ly++;
					}
					gbz80_ppu_update_stat_register(ppu, 0, ly);
				}
			}
			else {
				if (time_span <= NUM_DOTS_ZERO) {
					if (time_span == NUM_DOTS_ZERO) {
						ly++;
					}
					gbz80_ppu_update_stat_register(ppu, 1, ly);
					
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
	gbz80_cpu_memory_write8(&ppu->instance->cpu, 0xFF44, ly);

	uint8_t lyc = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF4A);
	uint8_t stat = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF41);

	common_change8_bit(&stat, 0, mode & 0x1);
	common_change8_bit(&stat, 1, (mode >> 1) & 0x1);
	common_change8_bit(&stat, 2, ly == lyc);

	gbz80_cpu_memory_write8(&ppu->instance->cpu, 0xFF41, stat);
}

void gbz80_ppu_draw_background(gbz80_ppu_t* ppu, uint8_t ly) {
	uint8_t scy = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF42);
	uint8_t scx = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF43);

	uint8_t start_memory_index = (scy + (ly / 8)) * 32 + scx;
	for (uint8_t tile_index = 0; tile_index < 20; tile_index++) {
		uint8_t current_tile_index = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0x9800 + (start_memory_index + tile_index));
		uint8_t tile_line_index = ly % 8;
		uint8_t low_byte = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0x8000 + current_tile_index);
		uint8_t high_byte = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0x8000 + (current_tile_index + 1));

		for (uint8_t bit_index = 8; bit_index > 0; bit_index--) {
			uint8_t colorID = (common_get8_bit(high_byte, bit_index - 1) << 1) | common_get8_bit(low_byte, bit_index - 1);

			ppu->lcd[(ly * 160) + (tile_index * 8 + bit_index)] = colorID;
		}

		current_tile_index += 2;
	}
}
