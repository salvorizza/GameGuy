#include "common.h"

void common_change8_bit(uint8_t* bitset, uint8_t bit_pos, uint8_t bit_val) {
	*bitset = (*bitset & ~(1 << (uint8_t)bit_pos)) | (bit_val << (uint8_t)bit_pos);
}

void common_toggle8_bit(uint8_t* bitset, uint8_t bit_pos)
{
	*bitset ^= (1 << bit_pos);
}

void common_set8_bit(uint8_t* bitset, uint8_t bit_pos)
{
	*bitset |= (1 << bit_pos);
}

void common_reset8_bit(uint8_t* bitset, uint8_t bit_pos)
{
	*bitset &= ~(1 << bit_pos);
}

uint8_t common_get8_bit(uint8_t bitset, uint8_t bit_pos)
{
	return (bitset >> bit_pos) & 0x1;
}
