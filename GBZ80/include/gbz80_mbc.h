#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

	#define RTC_FREQUENCY 32768000

	typedef uint8_t(*gbz80_mbc_write_t)(void*, uint16_t, uint8_t, uint32_t*);
	typedef uint8_t(*gbz80_mbc_read_t)(void*, uint16_t, uint8_t*, uint8_t* val);
	typedef void(*gbz80_mbc_clock_t)(void*);

	typedef enum gbz80_mbc_type_t {
		GBZ80_MBC_TYPE_NULL,
		GBZ80_MBC_TYPE_1,
		GBZ80_MBC_TYPE_3,
		GBZ80_MBC_TYPE_5
	} gbz80_mbc_type_t;

	typedef struct gbz80_mbc_t {
		gbz80_mbc_type_t type;
		gbz80_mbc_write_t write_func;
		gbz80_mbc_read_t read_func;
		gbz80_mbc_clock_t clock_func;
		size_t ram_size, rom_size;
	} gbz80_mbc_t;

	typedef struct gbz80_mbc_null_t {
		gbz80_mbc_t base;
	} gbz80_mbc_null_t;

	typedef struct gbz80_mbc_001_t {
		gbz80_mbc_t base;
		uint8_t ramg;
		uint8_t bank_1;
		uint8_t bank_2;
		uint8_t mode;
	} gbz80_mbc_001_t;

	typedef struct {
		uint8_t seconds;
		uint8_t minutes;
		uint8_t hours;
		uint8_t days_low;
		uint8_t days_high;
	} gbz80_mbc_rtc_t;

	typedef struct gbz80_mbc_003_t {
		gbz80_mbc_t base;

		uint8_t ramg;
		uint8_t romb;
		uint8_t ramb_rtcr;

		gbz80_mbc_rtc_t rtc, rtc_latch;
		size_t rtc_clock;
		uint8_t prev_latch;
	} gbz80_mbc_003_t;

	typedef struct gbz80_mbc_005_t {
		gbz80_mbc_t base;

		uint8_t ramg;
		uint8_t romb0, romb1;
		uint8_t ramb;
	} gbz80_mbc_005_t;

	
	
	gbz80_mbc_t* gbz80_mbc_create(gbz80_mbc_type_t type, size_t rom_size, size_t ram_size);
	uint8_t gbz80_mbc_read(gbz80_mbc_t* mbc, uint16_t address, uint8_t* memory, uint8_t* val);
	uint8_t gbz80_mbc_write(gbz80_mbc_t* mbc, uint16_t address, uint8_t val, uint32_t* ram_address);
	void gbz80_mbc_clock(gbz80_mbc_t* mbc);
	void gbz80_mbc_release(gbz80_mbc_t* mbc);

	uint8_t gbz80_mbc_null_read(void* mbc_null, uint16_t address, uint8_t* memory, uint8_t* val);
	uint8_t gbz80_mbc_null_write(void* mbc_null, uint16_t address, uint8_t val, uint32_t* ram_address);
	void gbz80_mbc_null_clock(void* mbc_null);

	uint8_t gbz80_mbc_001_read(void* mbc_001, uint16_t address, uint8_t* memory, uint8_t* val);
	uint8_t gbz80_mbc_001_write(void* mbc_001, uint16_t address, uint8_t val, uint32_t* ram_address);
	void gbz80_mbc_001_clock(void* mbc_001);

	uint8_t gbz80_mbc_003_read(void* mbc_003, uint16_t address, uint8_t* memory, uint8_t* val);
	uint8_t gbz80_mbc_003_write(void* mbc_003, uint16_t address, uint8_t val, uint32_t* ram_address);
	void gbz80_mbc_003_clock(void* mbc_003);

	uint8_t gbz80_mbc_005_read(void* mbc_005, uint16_t address, uint8_t* memory, uint8_t* val);
	uint8_t gbz80_mbc_005_write(void* mbc_005, uint16_t address, uint8_t val, uint32_t* ram_address);
	void gbz80_mbc_005_clock(void* mbc_005);

#ifdef __cplusplus
}
#endif