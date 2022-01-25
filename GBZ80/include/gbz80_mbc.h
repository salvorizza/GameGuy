#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

	typedef uint8_t(*gbz80_mbc_write_t)(uint16_t, uint8_t);
	typedef uint8_t(*gbz80_mbc_read_t)(uint16_t);

	typedef struct gbz80_mbc_t {
		uint8_t ram_enable;
		uint8_t ram_bank;
		uint8_t rom_bank;
		uint8_t mode;
		gbz80_mbc_write_t write_func;
		gbz80_mbc_read_t read_func;
	} gbz80_mbc_t;

#ifdef __cplusplus
}
#endif