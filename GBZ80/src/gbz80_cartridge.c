#include "gbz80_cartridge.h"

gbz80_cartridge_t* gbz80_cartridge_read_from_file(const char* rom_path) {
	gbz80_cartridge_t* out_rom = NULL;

	FILE* file_stream = fopen(rom_path, "rb");
	if (file_stream) {

		fseek(file_stream, 0x0000, SEEK_END);
		size_t code_size = ftell(file_stream) - (size_t)0x014F;
		fseek(file_stream, 0x0000, SEEK_SET);

		uint8_t* allocated_memory = (uint8_t*)malloc(sizeof(gbz80_cartridge_t) + sizeof(uint8_t) * code_size);
		if (allocated_memory != NULL) {
			memset(allocated_memory, 0, sizeof(gbz80_cartridge_t) + sizeof(uint8_t) * code_size);
			out_rom = (gbz80_cartridge_t*)allocated_memory;
			out_rom->code.data = allocated_memory + sizeof(gbz80_cartridge_t);
			out_rom->code.size = code_size;

			fread(&out_rom->rst, sizeof(gbz80_cartridge_rst_t), 1, file_stream);
			fread(&out_rom->interrupts, sizeof(gbz80_cartridge_interrupts_t), 1, file_stream);
			fread(out_rom->unused, sizeof(uint8_t) * BYTE(0x98), 1, file_stream);
			fread(out_rom->entry_point, sizeof(uint8_t) * BYTE(0x4), 1, file_stream);
			fread(&out_rom->header, sizeof(gbz80_cartridge_header_t), 1, file_stream);
			fread(out_rom->code.data, sizeof(uint8_t) * code_size, 1, file_stream);

			switch (out_rom->header.rom_size) {
				case 0x00:
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07:
				case 0x08:
					out_rom->rom_banks_size = KIBI(32) * (1 << out_rom->header.rom_size);
					break;

				case 0x52:
					out_rom->rom_banks_size = 72 * KIBI(4);
					break;
				case 0x53:
					out_rom->rom_banks_size = 80 * KIBI(4);
					break;
				case 0x54:
					out_rom->rom_banks_size = 96 * KIBI(4);
					break;
			}
			out_rom->rom_banks = (uint8_t*)malloc(out_rom->rom_banks_size);
			if(out_rom->rom_banks) memset(out_rom->rom_banks, 0, out_rom->rom_banks_size);

			switch (out_rom->header.ram_size) {
				case GBZ80_RAM_SIZE_NONE:
					out_rom->ram_banks_size = 0;
					break;
				case GBZ80_RAM_SIZE_8K:
					out_rom->ram_banks_size = KIBI(8);
					break;
				case GBZ80_RAM_SIZE_32K:
					out_rom->ram_banks_size = KIBI(32);
					break;
				case GBZ80_RAM_SIZE_64K:
					out_rom->ram_banks_size = KIBI(64);
					break;
				case GBZ80_RAM_SIZE_128K:
					out_rom->ram_banks_size = KIBI(128);
					break;
			}


			fseek(file_stream, 0, SEEK_SET);
			if (out_rom->rom_banks) fread(out_rom->rom_banks, out_rom->rom_banks_size + out_rom->header.cartridge_type, 1, file_stream);

			switch (out_rom->header.cartridge_type)
			{
				case GBZ80_CARTRIDGE_TYPE_ROM_ONLY:
					out_rom->mbc = gbz80_mbc_create(GBZ80_MBC_TYPE_NULL, out_rom->rom_banks_size, out_rom->ram_banks_size);
					break;

				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1_AND_RAM:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1_AND_RAM_AND_BATT:
					out_rom->mbc = gbz80_mbc_create(GBZ80_MBC_TYPE_1, out_rom->rom_banks_size, out_rom->ram_banks_size);
					break;

				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_RAM:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_RAM_AND_BATT:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_TIMER_AND_BATT:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC3_AND_TIMER_AND_RAM_AND_BATT:
					out_rom->mbc = gbz80_mbc_create(GBZ80_MBC_TYPE_3, out_rom->rom_banks_size, out_rom->ram_banks_size);
					break;

				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RAM:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RAM_AND_BATT:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RUMBLE:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RUMBLE_AND_SRAM:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC5_AND_RUMBLE_AND_SRAM_AND_BATT:
					out_rom->mbc = gbz80_mbc_create(GBZ80_MBC_TYPE_5, out_rom->rom_banks_size, out_rom->ram_banks_size);
					break;

				default: {
					assert(0 && "MBC Not managed ");
				}
			}


			if (out_rom->ram_banks_size != 0) {
				out_rom->ram_banks = (uint8_t*)malloc(out_rom->ram_banks_size);
				if (out_rom->ram_banks) memset(out_rom->ram_banks, 0, out_rom->ram_banks_size);
			}
			else {
				out_rom->ram_banks = NULL;
			}
		}

		fclose(file_stream);
	}
	else {
		return NULL;
	}

	return out_rom;
}

uint8_t gbz80_cartridge_read(gbz80_cartridge_t* cartridge, uint16_t address, uint8_t* out_val) {
	uint32_t read_res;
	if (cartridge) {
		uint8_t* memory = NULL;
		if (address >= 0x0000 && address <= 0x7FFF) {
			memory = cartridge->rom_banks;
		} else if (address >= 0xA000 && address <= 0xBFFF) {
			memory = cartridge->ram_banks;
		}

		read_res = gbz80_mbc_read(cartridge->mbc, address, memory, out_val);
		if (read_res == 2) {
			*out_val = 0xFF;
			return 1;
		}

		return read_res;
	}
	return 0;
}

uint8_t gbz80_cartridge_write(gbz80_cartridge_t* cartridge, uint16_t address, uint8_t val) {
	uint32_t ram_address;
	if (cartridge && gbz80_mbc_write(cartridge->mbc, address, val, &ram_address)) {
		if (address >= 0xA000 && address <= 0xBFFF) {
			if(cartridge->ram_banks)
				cartridge->ram_banks[ram_address] = val;
		}
	}
	return 0;
}

void gbz80_cartridge_destroy(gbz80_cartridge_t* rom) {
	if (rom->ram_banks) {
		free(rom->ram_banks);
		rom->ram_banks = NULL;
	}
	if (rom->rom_banks) {
		free(rom->rom_banks);
		rom->rom_banks = NULL;
	}
	gbz80_mbc_release(rom->mbc);
	free(rom);
	rom = NULL;
}
