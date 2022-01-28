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

			switch (out_rom->header.cartridge_type)
			{
				case GBZ80_CARTRIDGE_TYPE_ROM_ONLY:
					out_rom->mbc = gbz80_mbc_create(GBZ80_MBC_TYPE_NULL);
					break;

				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1_AND_RAM:
				case GBZ80_CARTRIDGE_TYPE_ROM_AND_MBC1_AND_RAM_AND_BATT:
					out_rom->mbc = gbz80_mbc_create(GBZ80_MBC_TYPE_1);
					break;
			}

			if (out_rom->header.rom_size != GBZ80_ROM_SIZE_32K) {
				out_rom->rom_banks_size = KIBI(32) << (size_t)out_rom->header.rom_size;
				out_rom->rom_banks = (uint8_t*)malloc(out_rom->rom_banks_size);
				memset(out_rom->rom_banks, 0, out_rom->rom_banks_size);
			} else {
				out_rom->rom_banks = NULL;
				out_rom->rom_banks_size = 0;
			}

			switch (out_rom->header.rom_size) {
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

			if (out_rom->ram_banks_size != 0) {
				out_rom->ram_banks = (uint8_t*)malloc(out_rom->ram_banks_size);
				memset(out_rom->ram_banks, 0, out_rom->ram_banks_size);
			} else {
				out_rom->ram_banks = NULL;
			}

			fseek(file_stream, 0, SEEK_SET);
			fread(out_rom->rom_banks, out_rom->rom_banks_size, 1, file_stream);

			

		}

		fclose(file_stream);
	}
	else {
		return NULL;
	}

	return out_rom;
}

uint8_t gbz80_cartridge_read(gbz80_cartridge_t* cartridge, uint16_t address, uint8_t* out_val) {
	uint32_t mapped_address;
	if (cartridge && cartridge->mbc->read_func(cartridge->mbc, address, &mapped_address) == 1) {
		if (address >= 0x0000 && address <= 0x7FFF) {
			*out_val = cartridge->rom_banks[mapped_address];
		} else if (address >= 0xA000 && address <= 0xBFFF) {
			*out_val = cartridge->ram_banks[mapped_address];
		}
		return 1;
	}
	return 0;
}

uint8_t gbz80_cartridge_write(gbz80_cartridge_t* cartridge, uint16_t address, uint8_t val) {
	uint32_t ram_address;
	if (cartridge && cartridge->mbc->write_func(cartridge->mbc, address, val, &ram_address)) {
		if (address >= 0xA000 && address <= 0xBFFF) {
			cartridge->ram_banks[ram_address] = val;
		}
	}
	return 0;
}

void gbz80_cartridge_destroy(gbz80_cartridge_t* rom) {
	if (rom->ram_banks) {
		free(rom->ram_banks);
	}
	if (rom->rom_banks) {
		free(rom->rom_banks);
	}
	gbz80_mbc_release(rom->mbc);
	free(rom);
	rom = NULL;
}
