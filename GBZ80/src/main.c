/*#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <Windows.h>
#include "gbz80.h"

#define GBZ80_CLOCK_HERTZ 4194304

int64_t get_frequency() {
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	return Frequency.QuadPart;
}
int64_t get_current_ticks() {
	LARGE_INTEGER Time;
	QueryPerformanceCounter(&Time);
	return Time.QuadPart;
}

int main(int argc, char** argv) {
	gbz80_t* instance = gbz80_create();
	gbz80_cartridge_t* cartridge = gbz80_cartridge_read_from_file("roms/tetris.gb");
	gbz80_load_cartridge(instance, cartridge);


	int64_t frequency = get_frequency();
	int64_t start_time = get_current_ticks();
	int64_t num_cycles = 0;
	while (1) {
		if (num_cycles == GBZ80_CLOCK_HERTZ) {
			int64_t millisec = (1000 * (get_current_ticks() - start_time)) / frequency;
			Sleep(1000 - millisec);
			start_time = get_current_ticks();
			num_cycles = 0;
		}

		//Emulare
		num_cycles += gbz80_step(instance);

		num_cycles += 1;
	}

	gbz80_cartridge_destroy(cartridge);
	gbz80_destroy(instance);

	return 0;
}

int main(int argc, char** argv) {
	gbz80_t* instance = gbz80_create();

	FILE* f = fopen("roms/test_program.bin","rb");
	fseek(f, 0, SEEK_END);
	size_t total_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fread(instance->memory_map, total_size, 1, f);
	instance->cpu.registers.PC = 0x0000;
	fclose(f);

	int64_t frequency = get_frequency();
	int64_t start_time = get_current_ticks();
	int64_t num_cycles = 0;
	while (1) {
		if (num_cycles == GBZ80_CLOCK_HERTZ) {
			DWORD millisec = (1000 * (get_current_ticks() - start_time)) / frequency;
			Sleep(1000 - millisec);
			start_time = get_current_ticks();
			num_cycles = 0;
		}

		num_cycles += gbz80_step(instance);
	}

	gbz80_destroy(instance);
	return 0;

}*/

