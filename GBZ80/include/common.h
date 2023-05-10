#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#define GBZ80_CLOCK_HERTZ 0x400000

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <string.h>

#define BYTE(x) (x)
#define KIBI(x) (x * BYTE(1024))
#define MEBI(x) (x * KIBI(1024))

	typedef struct gbz80_timer_t {
		size_t counter;
		size_t period;
	} gbz80_timer_t;

	static __forceinline void gbz80_init_timer(gbz80_timer_t* timer, size_t period) {
		timer->period = period;
		timer->counter = period;
	}

	static __forceinline uint8_t gbz80_update_timer(gbz80_timer_t* timer) {
		timer->counter--;
		if (timer->counter == 0) {
			timer->counter = timer->period;
			return 1;
		}
		return 0;
	}

	static __forceinline void common_change8_bit(uint8_t* bitset, uint8_t bit_pos, uint8_t bit_val) {
		*bitset = (*bitset & ~(1 << (uint8_t)bit_pos)) | (bit_val << (uint8_t)bit_pos);
	}

	static __forceinline void common_change8_bit_range(uint8_t* bitset, uint8_t bit_pos_start, uint8_t bit_pos_end, uint8_t val) {
		uint8_t mask = 0xFF >> (8 - bit_pos_end + bit_pos_start - 1);
		uint8_t write_mask = val ? mask : 0;
		*bitset = (*bitset & ~(mask << (uint8_t)bit_pos_start)) | (write_mask & val);
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

	static __forceinline uint16_t common_get16_bit(uint16_t bitset, uint8_t bit_pos)
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
