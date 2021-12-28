#pragma once

#include "common.h"

typedef enum gbz80_color_flag_t {
	GBZ80_COLOR_FLAG_GB_AND_CGB = 0x80,
	GBZ80_COLOR_FLAG_CGB_ONLY = 0xC0
} gbz80_color_flag_t;
typedef enum gbz80_super_gameboy_flag_t {
	GBZ80_SUPER_GAMEBOY_FLAG_NORMAL = 0x00,
	GBZ80_SUPER_GAMEBOY_FLAG_SGB = 0x03
} gbz80_super_gameboy_flag_t;
typedef enum gbz80_cartridge_type_t {
	GBZ80_CARTRIDGE_TYPE_ROM_ONLY = 0x00,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1 = 0x01,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1_AND_RAM = 0x02,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1_AND_RAM_AND_BATT = 0x03,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC2 = 0x05,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC2_AND_BATTERY = 0x06,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_RAM = 0x08,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_RAM_AND_BATTERY = 0x09,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MMM01 = 0x0B,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MMM01_AND_SRAM = 0xC,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MMM01_AND_SRAM_AND_BATT = 0xD,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_TIMER_AND_BATT = 0xF,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_TIMER_AND_RAM_AND_BATT = 0x10,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3 = 0x11,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_RAM = 0x12,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_RAM_AND_BATT = 0x013,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5 = 0x019,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RAM = 0x01A,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RAM_AND_BATT = 0x01B,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RUMBLE = 0x1C,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RUMBLE_AND_SRAM = 0x1D,
	GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RUMBLE_AND_SRAM_AND_BATT = 0x1E,
	GBZ80_CARTRIDGE_TYPE_POCKET_CAMERA = 0x1F,
	GBZ80_CARTRIDGE_TYPE_BANDAI_TAMA5 = 0xFD,
	GBZ80_CARTRIDGE_TYPE_HUDSON_HUC_3 = 0xFE,
	GBZ80_CARTRIDGE_TYPE_HUDSON_HUC_1 = 0xFF
} gbz80_cartridge_type_t;
typedef enum gbz80_rom_size_t {
	GBZ80_ROM_SIZE_32K = 0x00,
	GBZ80_ROM_SIZE_64K = 0x01,
	GBZ80_ROM_SIZE_128K = 0x02,
	GBZ80_ROM_SIZE_256K = 0x03,
	GBZ80_ROM_SIZE_512K = 0x04,
	GBZ80_ROM_SIZE_1M = 0x05,
	GBZ80_ROM_SIZE_2M = 0x06,
	GBZ80_ROM_SIZE_1_1M = 0x52,
	GBZ80_ROM_SIZE_1_2M = 0x53,
	GBZ80_ROM_SIZE_1_5M = 0x54,
} gbz80_rom_size_t;
typedef enum gbz80_ram_size_t {
	GBZ80_RAM_SIZE_NONE = 0x00,
	GBZ80_RAM_SIZE_16K = 0x01,
	GBZ80_RAM_SIZE_64K = 0x02,
	GBZ80_RAM_SIZE_256K = 0x03,
	GBZ80_RAM_SIZE_1M = 0x04
} gbz80_ram_size_t;
typedef struct gbz80_cartridge_rst_t {
	uint8_t rst_adresses_0[BYTE(0x8)];
	uint8_t rst_adresses_1[BYTE(0x8)];
	uint8_t rst_adresses_2[BYTE(0x8)];
	uint8_t rst_adresses_3[BYTE(0x8)];
	uint8_t rst_adresses_4[BYTE(0x8)];
	uint8_t rst_adresses_5[BYTE(0x8)];
	uint8_t rst_adresses_6[BYTE(0x8)];
	uint8_t rst_adresses_7[BYTE(0x8)];
} gbz80_cartridge_rst_t;
typedef struct gbz80_cartridge_interrupts_t {
	uint8_t interrupt_vblank[BYTE(0x8)];
	uint8_t interrupt_lcd_stat[BYTE(0x8)];
	uint8_t interrupt_timer[BYTE(0x8)];
	uint8_t interrupt_serial[BYTE(0x8)];
	uint8_t interrupt_joypad[BYTE(0x8)];
} gbz80_cartridge_interrupts_t;
typedef struct gbz80_cartridge_header_t {
	uint8_t nintendo_logo[BYTE(0x30)];
	uint8_t game_name[BYTE(0xF)];
	uint8_t color_flag;
	uint16_t manufacturer_code;
	uint8_t super_gameboy_flag;
	uint8_t cartridge_type;
	uint8_t rom_size;
	uint8_t ram_size;
	uint8_t destination_code;
	uint8_t licensee_code;
	uint8_t mask_rom_version_number;
	uint8_t header_checksum;
	uint16_t global_checksum;
} gbz80_cartridge_header_t;
typedef struct gbz80_cartridge_code_t {
	uint8_t* data;
	size_t size;
} gbz80_cartridge_code_t;
typedef struct gbz80_cartridge_t {
	gbz80_cartridge_rst_t rst;
	gbz80_cartridge_interrupts_t interrupts;
	uint8_t unused[BYTE(0x98)];
	uint8_t entry_point[BYTE(0x4)];
	gbz80_cartridge_header_t header;
	gbz80_cartridge_code_t code;
} gbz80_cartridge_t;

gbz80_cartridge_t* gbz80_cartridge_read_from_file(const char* rom_path);
void gbz80_cartridge_destroy(gbz80_cartridge_t* rom);

