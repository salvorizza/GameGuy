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

	#define BYTE(x) (x)
	#define KIBI(x) (1024llu * BYTE(x))
	#define MIBI(x) (1024llu * KIBI(x))

	static __forceinline void common_change8_bit(uint8_t* bitset, uint8_t bit_pos, uint8_t bit_val) {
		*bitset = (*bitset & ~(1 << (uint8_t)bit_pos)) | (bit_val << (uint8_t)bit_pos);
	}

	static __forceinline void common_change8_bit_range(uint8_t* bitset, uint8_t bit_pos_start, uint8_t bit_pos_end, uint8_t val) {
		uint8_t mask = 0xFF >> (8 - bit_pos_end + bit_pos_start - 1);
		*bitset = (*bitset & ~(mask << (uint8_t)bit_pos_start)) | (val << (uint8_t)bit_pos_start);
	}

	static __forceinline void common_toggle8_bit(uint8_t* bitset, uint8_t bit_pos)
	{
		*bitset ^= (1 << bit_pos);
	}

	static __forceinline void common_set8_bit(uint8_t* bitset, uint8_t bit_pos)
	{
		*bitset |= (1 << bit_pos);
	}

	static __forceinline void common_reset8_bit(uint8_t* bitset, uint8_t bit_pos)
	{
		*bitset &= ~(1 << bit_pos);
	}

	static __forceinline uint8_t common_get8_bit(uint8_t bitset, uint8_t bit_pos)
	{
		return (bitset >> bit_pos) & 0x1;
	}

	static __forceinline uint8_t common_get8_bit_range(uint8_t bitset, uint8_t bit_pos_start, uint8_t bit_pos_end) {
		uint8_t mask = 0xFF >> (8 - bit_pos_end + bit_pos_start - 1);
		return (bitset >> bit_pos_start) & mask;
	}

#ifdef __cplusplus
}
#endif
