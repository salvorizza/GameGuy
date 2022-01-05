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

			if (ly >= 0 && ly <= 143) {
				if (time_span <= NUM_DOTS_TWO) {
					gbz80_ppu_update_stat_register(ppu, 2, ly);
				}
				else if (time_span <= NUM_DOTS_THREE) {
					uint8_t scy = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF42);
					uint8_t scx = gbz80_cpu_memory_read8(&ppu->instance->cpu, 0xFF43);

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
