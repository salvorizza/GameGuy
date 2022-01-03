#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	#include <stdint.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <memory.h>
	#include <assert.h>
	#include <string.h>

	#define BYTE(x) x
	#define KIBI(x) 1024 * BYTE(x)
	#define MIBI(x) 1024 * KIBI(x)

	void common_change8_bit(uint8_t* bitset, uint8_t bit_pos, uint8_t bit_val);
	void common_toggle8_bit(uint8_t* bitset, uint8_t bit_pos);
	void common_set8_bit(uint8_t* bitset, uint8_t bit_pos);
	void common_reset8_bit(uint8_t* bitset, uint8_t bit_pos);

	uint8_t common_get8_bit(uint8_t bitset, uint8_t bit_pos);

#ifdef __cplusplus
}
#endif
