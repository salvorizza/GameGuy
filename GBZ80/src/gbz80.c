#include "gbz80.h"

gbz80_t* gbz80_create() {
	gbz80_t* instance = (gbz80_t*)malloc(sizeof(gbz80_t));
	if (instance != NULL){
		memset(instance->memory_map, 0, GBZ80_MEMORY_SIZE);
	}
	return instance;
}

void gbz80_init(gbz80_t* instance)
{
	gbz80_cpu_init(&instance->cpu, instance->memory_map, GBZ80_MEMORY_SIZE);
}


void gbz80_load_cartridge(gbz80_t* instance, gbz80_cartridge_t* rom)
{
	if (rom->header.cartridge_type == GBZ80_CARTRIDGE_TYPE_ROM_ONLY) {
		memcpy(&instance->memory_map[0x0000], rom, sizeof(gbz80_cartridge_t) - sizeof(gbz80_cartridge_code_t));
		memcpy(&instance->memory_map[0x0150], rom->code.data, rom->code.size);
		instance->cartridge_code_size = rom->code.size;
	}
	else {
		assert(0 && "Rom not supported yet");
	}
}

size_t gbz80_step(gbz80_t* instance) {
	return gbz80_cpu_step(&instance->cpu);
}

void gbz80_destroy(gbz80_t* instance)
{
	free(instance);
	instance = NULL;
}