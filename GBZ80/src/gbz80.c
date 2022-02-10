#include "gbz80.h"

gbz80_t* gbz80_create() {
	gbz80_t* instance = (gbz80_t*)malloc(sizeof(gbz80_t));
	return instance;
}

uint8_t gbz80_memory_read8(gbz80_t* instance, uint16_t address) {
	uint8_t val;
	
	if (instance->bootstrap_mode == 1 && address <= 0xFF) {
		return instance->bootstrap_rom[address];
	} else if (gbz80_cartridge_read(instance->inserted_cartridge, address, &val)) {
		return val;
	} else {
		switch (address) {
			default: {
				uint8_t val = instance->memory_map[address];
				gbz80_cpu_memory_read(&instance->cpu, address, &val);
				gbz80_apu_memory_read(&instance->apu, address, &val);
				return val;
			}
		}
				
	}
}

void gbz80_memory_write8(gbz80_t* instance, uint16_t address, uint8_t val) {
	if (instance->bootstrap_mode == 1 && address <= 0xFF) {
		instance->bootstrap_rom[address] = val;
	}
	else {
		if (address == 0xFF50 && instance->bootstrap_mode == 1 && val == 1) {
			instance->bootstrap_mode = 0;
		}

		if (instance->bootstrap_mode == 0 && gbz80_cartridge_write(instance->inserted_cartridge, address, val)) {
			
		} else {
			uint8_t current_value = instance->memory_map[address];

			uint8_t cpu_write_flag = gbz80_cpu_memory_write(&instance->cpu, address, current_value, &val);
			uint8_t apu_write_flag = gbz80_apu_memory_write(&instance->apu, address, current_value, &val);

			instance->memory_map[address] = val;
		}
	}

}

uint16_t gbz80_memory_read16(gbz80_t* instance, uint16_t address) {
	return (uint16_t)gbz80_memory_read8(instance, address + 1) << 8 | gbz80_memory_read8(instance, address);
}

void gbz80_memory_write16(gbz80_t* instance, uint16_t address, uint16_t val) {
	gbz80_memory_write8(instance, address, (uint8_t)(val & 0xFF));
	gbz80_memory_write8(instance, address + 1, (uint8_t)((val >> 8) & 0xFF));
}

void gbz80_set_sample_rate(gbz80_t* instance, size_t sample_rate) {
	gbz80_init_timer(&instance->apu.sample_timer, GBZ80_APU_FREQ / sample_rate);
}

void gbz80_init(gbz80_t* instance, const char* bios_path) {
	FILE* f = fopen(bios_path, "rb");
	if (f) {
		fread(instance->bootstrap_rom, BYTE(256), 1, f);

		fclose(f);
	}

	memset(instance->memory_map, 0, GBZ80_MEMORY_SIZE);
	instance->bootstrap_mode = 1;
	instance->cartridge_code_size = 0;
	instance->inserted_cartridge = NULL;
	gbz80_apu_init(&instance->apu, instance);
	gbz80_ppu_init(&instance->ppu, instance);
	gbz80_cpu_init(&instance->cpu, instance);
	gbz80_joypad_init(&instance->joypad, instance);
}

void gbz80_load_cartridge(gbz80_t* instance, gbz80_cartridge_t* rom)
{
	instance->inserted_cartridge = rom;
	if (rom->header.cartridge_type == GBZ80_CARTRIDGE_TYPE_ROM_ONLY) {
		memcpy(&instance->memory_map[0x0000], rom, sizeof(gbz80_cartridge_t) - sizeof(gbz80_cartridge_code_t));
		memcpy(&instance->memory_map[0x0150], rom->code.data, rom->code.size);
		instance->cartridge_code_size = rom->code.size;
	}
}

void gbz80_clock(gbz80_t* instance){
	gbz80_cpu_clock(&instance->cpu);
	gbz80_ppu_clock(&instance->ppu);
	gbz80_apu_clock(&instance->apu);
}

size_t gbz80_utility_get_num_cycles_from_seconds(gbz80_t* instance, double seconds)
{
	return (size_t)(seconds * GBZ80_CLOCK_HERTZ);
}

void gbz80_destroy(gbz80_t* instance)
{
	free(instance);
	instance = NULL;
}

uint8_t gbz80_memory_read_internal(gbz80_t* instance, uint16_t address)
{
	return instance->memory_map[address];
}

void gbz80_memory_write_internal(gbz80_t* instance, uint16_t address, uint8_t val)
{
	instance->memory_map[address] = val;
}
