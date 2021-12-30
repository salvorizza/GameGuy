#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "gbz80_cartridge.h"
#include "gbz80_cpu.h"

#define GBZ80_MEMORY_SIZE KIBI(64)

	typedef struct gbz80_video_ram_t {
		union {
			uint8_t memory[KIBI(8)];
			struct {
				uint8_t tiles_sprites[KIBI(4)];
				uint8_t tiles_alt[KIBI(2)];
				uint8_t tilemap_1[KIBI(1)];
				uint8_t tilemap_2[KIBI(1)];
			};
		};
	} gbz80_video_ram_t;

	typedef struct gbz80_t {
		union {
			uint8_t memory_map[GBZ80_MEMORY_SIZE];
			struct {
				uint8_t rom_bank_0[KIBI(0x10)];
				uint8_t switchable_rom_bank[KIBI(0x10)];
				gbz80_video_ram_t video_ram;
				uint8_t switchable_ram_bank[KIBI(0x8)];
				uint8_t internal_8k_ram[KIBI(0x8)];
				uint8_t echo_internal_8k_ram[KIBI(0x7) + BYTE(0x200)];
				uint8_t sprite_attrib_memory[BYTE(0xA0)];
				uint8_t io_unused[BYTE(0x60)];
				uint8_t io_ports[BYTE(0x4C)];
				uint8_t io_unused_2[BYTE(0x34)];
				uint8_t internal_ram[BYTE(0x7F)];
				uint8_t interrupt_enable_registers[BYTE(0x1)];
			};
		};
		gbz80_cpu_t cpu;
	} gbz80_t;

	gbz80_t* gbz80_create();
	void gbz80_load_cartridge(gbz80_t* instance, gbz80_cartridge_t* rom);
	size_t gbz80_step(gbz80_t* instance);
	void gbz80_destroy(gbz80_t* rom);

#ifdef __cplusplus
}
#endif
