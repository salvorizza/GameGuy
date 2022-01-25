#include "gbz80_mbc.h"


gbz80_mbc_t* gbz80_mbc_create(gbz80_mbc_type_t type) {
	gbz80_mbc_t* mbc = NULL;

	switch (type) {
	case GBZ80_MBC_TYPE_NULL: {
		mbc = (gbz80_mbc_t*)malloc(sizeof(gbz80_mbc_null_t));
		if (mbc) {
			mbc->read_func = &gbz80_mbc_null_read;
			mbc->write_func = &gbz80_mbc_null_write;
		}
		break;
	}

	case GBZ80_MBC_TYPE_1: {
		mbc = (gbz80_mbc_t*)malloc(sizeof(gbz80_mbc_001_t));
		if (mbc) {
			mbc->read_func = &gbz80_mbc_001_read;
			mbc->write_func = &gbz80_mbc_001_write;
		}
		break;
	}
	}

	mbc->type = type;

	return mbc;
}

uint8_t gbz80_mbc_read(gbz80_mbc_t* mbc, uint16_t address, uint32_t* mapped_address) {
	return mbc->read_func(mbc, address, mapped_address);
}

uint8_t gbz80_mbc_write(gbz80_mbc_t* mbc, uint16_t address, uint8_t val) {
	return mbc->write_func(mbc, address, val);
}


void gbz80_mbc_release(gbz80_mbc_t* mbc) {
	free(mbc);
}

uint8_t gbz80_mbc_null_read(void* mbc_null, uint16_t address, uint32_t* mapped_address) {
	gbz80_mbc_null_t* mbc = (gbz80_mbc_null_t*)mbc_null;

	return 0;
}

uint8_t gbz80_mbc_null_write(void* mbc_null, uint16_t address, uint8_t val) {
	gbz80_mbc_null_t* mbc = (gbz80_mbc_null_t*)mbc_null;

	return 0;
}

uint8_t gbz80_mbc_001_read(void* mbc_001, uint16_t address, uint32_t* mapped_address){
	gbz80_mbc_001_t* mbc = (gbz80_mbc_001_t*)mbc_001;

	if (address >= 0x0000 && address <= 0x3FFF) {
		return 0;
	} else if (address >= 0x4000 && address <= 0x7FFF) {
		//Read from internal rom using rom bank
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		//Read from internal ram using ram bank
	}


	return 0;
}

uint8_t gbz80_mbc_001_write(void* mbc_001, uint16_t address, uint8_t val) {
	gbz80_mbc_001_t* mbc = (gbz80_mbc_001_t*)mbc_001;

	if (address >= 0x0000 && address <= 0x1FFF) {
		mbc->ram_enable = (val & 0x0F) == 0x0A;
	} else if (address >= 0x2000 && address <= 0x3FFF) {
		mbc->rom_bank = val & 0x1F;
		if (mbc->rom_bank == 0x00) {
			mbc->rom_bank = 0x1;
		}
	} else if (address >= 0x4000 && address <= 0x5FFF) {
		if (mbc->mode == 1) {
			mbc->ram_bank = val & 0x3;
		} else {
			mbc->rom_bank = (val & 0x3) << 5 | mbc->rom_bank;
		}
	} else if (address >= 0x6000 && address <= 0x7FFF) {
		mbc->mode = val & 0x1;
	}

	return 0;
}
