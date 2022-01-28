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
		gbz80_mbc_001_t* mbc_001 = (gbz80_mbc_001_t*)malloc(sizeof(gbz80_mbc_001_t));
		if (mbc_001) {
			mbc_001->base.read_func = &gbz80_mbc_001_read;
			mbc_001->base.write_func = &gbz80_mbc_001_write;
			mbc_001->mode = 0;
			mbc_001->ram_bank = 0;
			mbc_001->rom_bank = 0;
			mbc_001->ram_enable = 0;
			mbc = (gbz80_mbc_t*)mbc_001;
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

uint8_t gbz80_mbc_write(gbz80_mbc_t* mbc, uint16_t address, uint8_t val, uint32_t* ram_address) {
	return mbc->write_func(mbc, address, val, ram_address);
}


void gbz80_mbc_release(gbz80_mbc_t* mbc) {
	free(mbc);
}

uint8_t gbz80_mbc_null_read(void* mbc_null, uint16_t address, uint32_t* mapped_address) {
	gbz80_mbc_null_t* mbc = (gbz80_mbc_null_t*)mbc_null;

	return 0;
}

uint8_t gbz80_mbc_null_write(void* mbc_null, uint16_t address, uint8_t val, uint32_t* ram_address) {
	gbz80_mbc_null_t* mbc = (gbz80_mbc_null_t*)mbc_null;

	return 0;
}

uint8_t gbz80_mbc_001_read(void* mbc_001, uint16_t address, uint32_t* mapped_address){
	gbz80_mbc_001_t* mbc = (gbz80_mbc_001_t*)mbc_001;

	if (address >= 0x0000 && address <= 0x3FFF) {
		*mapped_address = address;
		return 1;
	} else if (address >= 0x4000 && address <= 0x7FFF) {
		*mapped_address = KIBI(16) * mbc->rom_bank + (address - 0x4000);
		return 1;
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ram_enable) {
			*mapped_address = KIBI(8) * mbc->ram_bank + (address - 0xA000);
			return 1;
		}
	}


	return 0;
}

uint8_t gbz80_mbc_001_write(void* mbc_001, uint16_t address, uint8_t val, uint32_t* ram_address) {
	gbz80_mbc_001_t* mbc = (gbz80_mbc_001_t*)mbc_001;

	if (address >= 0x0000 && address <= 0x1FFF) {
		mbc->ram_enable = (val & 0x0F) == 0x0A;
		return 1;
	} else if (address >= 0x2000 && address <= 0x3FFF) {
		mbc->rom_bank = val & 0x1F;

		/*switch (mbc->rom_bank) {
			case 0x00:
			case 0x20:
			case 0x40:
			case 0x60:
				mbc->rom_bank++;
				break;
		}*/
		return 1;
	} else if (address >= 0x4000 && address <= 0x5FFF) {
		if (mbc->mode == 1) {
			mbc->ram_bank = val & 0x3;
		} else {
			mbc->rom_bank = (val & 0x3) << 5 | mbc->rom_bank;
		}
		return 1;
	} else if (address >= 0x6000 && address <= 0x7FFF) {
		mbc->mode = val & 0x1;
		return 1;
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ram_enable) {
			*ram_address = KIBI(8) * mbc->ram_bank + (address - 0xA000);
			return 1;
		}
	}

	return 0;
}
