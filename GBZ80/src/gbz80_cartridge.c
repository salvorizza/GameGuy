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
		}

		fclose(file_stream);
	}
	else {
		return NULL;
	}

	return out_rom;
}

void gbz80_cartridge_destroy(gbz80_cartridge_t* rom) {
	free(rom);
	rom = NULL;
}
