#include "gbz80_mbc.h"


gbz80_mbc_t* gbz80_mbc_create(gbz80_mbc_type_t type, size_t rom_size, size_t ram_size) {
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
				mbc_001->bank_1 = 1;
				mbc_001->bank_2 = 0;
				mbc_001->ramg = 0;
				mbc = (gbz80_mbc_t*)mbc_001;
			}
			break;
		}

		case GBZ80_MBC_TYPE_3: {
			gbz80_mbc_003_t* mbc_003 = (gbz80_mbc_003_t*)malloc(sizeof(gbz80_mbc_003_t));
			if (mbc_003) {
				mbc_003->base.read_func = &gbz80_mbc_003_read;
				mbc_003->base.write_func = &gbz80_mbc_003_write;
				mbc_003->romb = 1;
				mbc_003->ramb = 0;
				mbc_003->rtc = 0;
				mbc_003->latch_clock = 0;
				mbc_003->latch = 0;
				mbc = (gbz80_mbc_t*)mbc_003;
			}
			break;
		}
	}

	if (mbc != NULL) {
		mbc->type = type;
		mbc->rom_size = rom_size;
		mbc->ram_size = ram_size;
	}

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

	if (address >= 0x0000 && address <= 0x3FFF) {
		*mapped_address = address;
		return 1;
	} else if (address >= 0x4000 && address <= 0x7FFF) {
		*mapped_address = address;
		return 1;
	}

	return 0;
}

uint8_t gbz80_mbc_null_write(void* mbc_null, uint16_t address, uint8_t val, uint32_t* ram_address) {
	gbz80_mbc_null_t* mbc = (gbz80_mbc_null_t*)mbc_null;

	return 0;
}

uint8_t gbz80_mbc_001_read(void* mbc_001, uint16_t address, uint32_t* mapped_address){
	gbz80_mbc_001_t* mbc = (gbz80_mbc_001_t*)mbc_001;


	if (address >= 0x0000 && address <= 0x3FFF) {
		uint32_t bank = mbc->mode ? mbc->bank_2 << 5 : 0;
		*mapped_address = bank << 14 | (uint32_t)(address & 0x3FFF);
		*mapped_address &= mbc->base.rom_size - 1;


		assert(*mapped_address < mbc->base.rom_size && "Out of bound ROM");
		return 1;
	} else if (address >= 0x4000 && address <= 0x7FFF) {
		uint32_t bank = mbc->bank_2 << 5 | mbc->bank_1;
		*mapped_address = bank << 14 | (uint32_t)(address & 0x3FFF);
		*mapped_address &= mbc->base.rom_size - 1;

		assert(*mapped_address < mbc->base.rom_size && "Out of bound ROM");

		return 1;
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			uint32_t bank = mbc->mode ? mbc->bank_2 : 0;
			*mapped_address = bank << 13 | (uint32_t)(address & 0x1FFF);
			*mapped_address &= mbc->base.ram_size - 1;

			return 1;
		}
		else {
			return 2;
		}
	}

	return 0;
}

uint8_t gbz80_mbc_001_write(void* mbc_001, uint16_t address, uint8_t val, uint32_t* ram_address) {
	gbz80_mbc_001_t* mbc = (gbz80_mbc_001_t*)mbc_001;

	if (address >= 0x0000 && address <= 0x1FFF) {
		mbc->ramg = (val & 0x0F) == 0x0A;
		return 1;
	} else if (address >= 0x2000 && address <= 0x3FFF) {
		mbc->bank_1 = val & 0x1F;

		switch (mbc->bank_1) {
			case 0x00:
			case 0x20:
			case 0x40:
			case 0x60:
				mbc->bank_1++;
				break;
		}

		return 1;
	} else if (address >= 0x4000 && address <= 0x5FFF) {
		mbc->bank_2 = val & 0x3;
		return 1;
	} else if (address >= 0x6000 && address <= 0x7FFF) {
		mbc->mode = val & 0x1;
		return 1;
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			uint32_t bank = mbc->mode ? mbc->bank_2 : 0;
			*ram_address = bank << 13 | (uint32_t)(address & 0x1FFF);
			*ram_address &= mbc->base.ram_size - 1;

			return 1;
		}
	}

	return 0;
}

uint8_t gbz80_mbc_003_read(void* mbc_003, uint16_t address, uint32_t* mapped_address) {
	gbz80_mbc_003_t* mbc = (gbz80_mbc_003_t*)mbc_003;

	if (address >= 0x0000 && address <= 0x3FFF) {
		*mapped_address = (uint32_t)(address & 0x3FFF);
		*mapped_address &= mbc->base.rom_size - 1;
		return 1;
	}
	else if (address >= 0x4000 && address <= 0x7FFF) {
		*mapped_address = mbc->romb << 14 | (uint32_t)(address & 0x3FFF);
		*mapped_address &= mbc->base.rom_size - 1;
		return 1;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			*mapped_address = mbc->ramb << 13 | (uint32_t)(address & 0x1FFF);
			*mapped_address &= mbc->base.ram_size - 1;

			return 1;
		} else {
			return 2;
		}
	}

	return 0;
}

uint8_t gbz80_mbc_003_write(void* mbc_003, uint16_t address, uint8_t val, uint32_t* ram_address) {
	gbz80_mbc_003_t* mbc = (gbz80_mbc_003_t*)mbc_003;

	if (address >= 0x0000 && address <= 0x1FFF) {
		mbc->ramg = (val & 0x0F) == 0x0A;
		return 1;
	}
	else if (address >= 0x2000 && address <= 0x3FFF) {
		mbc->romb = val ? val : 1;
		return 1;
	}
	else if (address >= 0x4000 && address <= 0x5FFF) {
		if (val >= 0x0 && val <= 0x3) {
			mbc->ramb = val & 0x3;
		} else if (val >= 0x8 && val <= 0xC) {
			mbc->rtc = val;
		}
		return 1;
	}
	else if (address >= 0x6000 && address <= 0x7FFF) {
		mbc->latch_clock = val;
		return 1;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			*ram_address = mbc->ramb << 13 | (uint32_t)(address & 0x1FFF);
			*ram_address &= mbc->base.ram_size - 1;

			return 1;
		}
	}

	return 0;
}
