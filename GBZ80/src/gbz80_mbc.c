#include "gbz80_mbc.h"


gbz80_mbc_t* gbz80_mbc_create(gbz80_mbc_type_t type, size_t rom_size, size_t ram_size) {
	gbz80_mbc_t* mbc = NULL;

	switch (type) {
		case GBZ80_MBC_TYPE_NULL: {
			mbc = (gbz80_mbc_t*)malloc(sizeof(gbz80_mbc_null_t));
			if (mbc) {
				mbc->read_func = &gbz80_mbc_null_read;
				mbc->write_func = &gbz80_mbc_null_write;
				mbc->clock_func = &gbz80_mbc_null_clock;
			}
			break;
		}

		case GBZ80_MBC_TYPE_1: {
			gbz80_mbc_001_t* mbc_001 = (gbz80_mbc_001_t*)malloc(sizeof(gbz80_mbc_001_t));
			if (mbc_001) {
				mbc_001->base.read_func = &gbz80_mbc_001_read;
				mbc_001->base.write_func = &gbz80_mbc_001_write;
				mbc_001->base.clock_func = &gbz80_mbc_001_clock;
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
				mbc_003->base.clock_func = &gbz80_mbc_003_clock;
				mbc_003->romb = 1;
				mbc_003->ramb_rtcr = 0;
				memset(&mbc_003->rtc, 0, sizeof(gbz80_mbc_rtc_t));
				memset(&mbc_003->rtc_latch, 0, sizeof(gbz80_mbc_rtc_t));
				mbc_003->prev_latch = 1;
				mbc_003->rtc_clock = 0;
				mbc = (gbz80_mbc_t*)mbc_003;
			}
			break;
		}

		case GBZ80_MBC_TYPE_5: {
			gbz80_mbc_005_t* mbc_005 = (gbz80_mbc_005_t*)malloc(sizeof(gbz80_mbc_005_t));
			if (mbc_005) {
				mbc_005->base.read_func = &gbz80_mbc_005_read;
				mbc_005->base.write_func = &gbz80_mbc_005_write;
				mbc_005->base.clock_func = &gbz80_mbc_005_clock;
				mbc_005->ramb = 0;
				mbc_005->ramg = 0;
				mbc_005->romb0 = 1;
				mbc_005->romb1 = 0;
				mbc = (gbz80_mbc_t*)mbc_005;
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

uint8_t gbz80_mbc_read(gbz80_mbc_t* mbc, uint16_t address, uint8_t* memory, uint8_t* val) {
	return mbc->read_func(mbc, address, memory, val);
}

uint8_t gbz80_mbc_write(gbz80_mbc_t* mbc, uint16_t address, uint8_t val, uint32_t* ram_address) {
	return mbc->write_func(mbc, address, val, ram_address);
}

void gbz80_mbc_clock(gbz80_mbc_t* mbc)
{
	mbc->clock_func(mbc);
}


void gbz80_mbc_release(gbz80_mbc_t* mbc) {
	free(mbc);
}

uint8_t gbz80_mbc_null_read(void* mbc_null, uint16_t address, uint8_t* memory, uint8_t* val) {
	gbz80_mbc_null_t* mbc = (gbz80_mbc_null_t*)mbc_null;

	if (address >= 0x0000 && address <= 0x3FFF) {
		*val = memory[address];
		return 1;
	} else if (address >= 0x4000 && address <= 0x7FFF) {
		*val = memory[address];
		return 1;
	}

	return 0;
}

uint8_t gbz80_mbc_null_write(void* mbc_null, uint16_t address, uint8_t val, uint32_t* ram_address) {
	gbz80_mbc_null_t* mbc = (gbz80_mbc_null_t*)mbc_null;

	return 0;
}

void gbz80_mbc_null_clock(void* mbc_null) {}

uint8_t gbz80_mbc_001_read(void* mbc_001, uint16_t address, uint8_t* memory, uint8_t* val){
	gbz80_mbc_001_t* mbc = (gbz80_mbc_001_t*)mbc_001;


	if (address >= 0x0000 && address <= 0x3FFF) {
		uint32_t bank = mbc->mode ? mbc->bank_2 << 5 : 0;
		uint32_t mapped_address = bank << 14 | (uint32_t)(address & 0x3FFF);
		mapped_address &= mbc->base.rom_size - 1;


		assert(mapped_address < mbc->base.rom_size && "Out of bound ROM");
		*val = memory[mapped_address];

		return 1;
	} else if (address >= 0x4000 && address <= 0x7FFF) {
		uint32_t bank = mbc->bank_2 << 5 | mbc->bank_1;
		uint32_t mapped_address = bank << 14 | (uint32_t)(address & 0x3FFF);
		mapped_address &= mbc->base.rom_size - 1;


		assert(mapped_address < mbc->base.rom_size && "Out of bound ROM");
		*val = memory[mapped_address];

		return 1;
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			uint32_t bank = mbc->mode ? mbc->bank_2 : 0;
			uint32_t mapped_address = bank << 13 | (uint32_t)(address & 0x1FFF);
			mapped_address &= mbc->base.ram_size - 1;

			*val = memory[mapped_address];
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

void gbz80_mbc_001_clock(void* mbc_001) {}

uint8_t gbz80_mbc_003_read(void* mbc_003, uint16_t address, uint8_t* memory, uint8_t* val) {
	gbz80_mbc_003_t* mbc = (gbz80_mbc_003_t*)mbc_003;

	if (address >= 0x0000 && address <= 0x3FFF) {
		uint32_t mapped_address = (uint32_t)(address & 0x3FFF);
		mapped_address &= mbc->base.rom_size - 1;
		*val = memory[mapped_address];
		return 1;
	}
	else if (address >= 0x4000 && address <= 0x7FFF) {
		uint32_t mapped_address = mbc->romb << 14 | (uint32_t)(address & 0x3FFF);
		mapped_address &= mbc->base.rom_size - 1;
		*val = memory[mapped_address];
		return 1;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			if (mbc->ramb_rtcr >= 0x0 && mbc->ramb_rtcr <= 0x3) {
				uint32_t mapped_address = mbc->ramb_rtcr << 13 | (uint32_t)(address & 0x1FFF);
				mapped_address &= mbc->base.ram_size - 1;
				*val = memory[mapped_address];
			}
			else if (mbc->ramb_rtcr >= 0x8 && mbc->ramb_rtcr <= 0xC) {
				switch (mbc->ramb_rtcr) {
					case 0x8:
						*val = mbc->rtc_latch.seconds;
						break;
					case 0x9:
						*val = mbc->rtc_latch.minutes;
						break;
					case 0xA:
						*val = mbc->rtc_latch.hours;
						break;
					case 0xB:
						*val = mbc->rtc_latch.days_low;
						break;
					case 0xC:
						*val = mbc->rtc_latch.days_high;
						break;
				}
			}

			
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
		mbc->ramb_rtcr = val & 0xF;
		return 1;
	}
	else if (address >= 0x6000 && address <= 0x7FFF) {
		if (mbc->prev_latch == 0 && (val & 0x1)) {
			mbc->rtc_latch = mbc->rtc;
		}
		mbc->prev_latch = val & 0x1;
		return 1;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			if (mbc->ramb_rtcr >= 0x0 && mbc->ramb_rtcr <= 0x3) {
				*ram_address = mbc->ramb_rtcr << 13 | (uint32_t)(address & 0x1FFF);
				*ram_address &= mbc->base.ram_size - 1;
			} else if (mbc->ramb_rtcr >= 0x8 && mbc->ramb_rtcr <= 0xC) {
				switch (mbc->ramb_rtcr) {
					case 0x8:
						mbc->rtc_latch.seconds = val & 0x3F;
						break;
					case 0x9:
						mbc->rtc_latch.minutes = val & 0x3F;
						break;
					case 0xA:
						mbc->rtc_latch.hours = val & 0x1F;
						break;
					case 0xB:
						mbc->rtc_latch.days_low = val & 0xFF;
						break;
					case 0xC:
						mbc->rtc_latch.days_high = val & 0xFF;
						break;
				}
			}
			return 1;
		}
	}

	return 0;
}

void gbz80_mbc_003_clock(void* mbc_003)
{
	gbz80_mbc_003_t* mbc = (gbz80_mbc_003_t*)mbc_003;

	for (uint32_t i = 0; i < 8; i++) {
		if (mbc->rtc_clock == RTC_FREQUENCY - 1) {
			mbc->rtc.seconds++;
			
			if (mbc->rtc.seconds == 60) {
				mbc->rtc.seconds = 0;
				mbc->rtc.minutes++;

				if (mbc->rtc.minutes == 60) {
					mbc->rtc.minutes = 0;
					mbc->rtc.hours++;

					if (mbc->rtc.hours == 24) {
						mbc->rtc.hours = 0;
						
						if (((uint16_t)mbc->rtc.days_low) + 1 > 0xFF) {
							if (common_get8_bit(mbc->rtc.days_high, 0)) {
								common_set8_bit(&mbc->rtc.days_high, 7);
								common_reset8_bit(&mbc->rtc.days_high, 0);
								mbc->rtc.days_low = 0;
							} else {
								common_set8_bit(&mbc->rtc.days_high, 0);
							}
						}
						else {
							mbc->rtc.days_low++;
						}
					}
				}
			}

			mbc->rtc_clock = 0;
		} else {
			mbc->rtc_clock++;
		}
	}
}

uint8_t gbz80_mbc_005_read(void* mbc_005, uint16_t address, uint8_t* memory, uint8_t* val)
{
	gbz80_mbc_005_t* mbc = (gbz80_mbc_005_t*)mbc_005;


	if (address >= 0x0000 && address <= 0x3FFF) {
		uint32_t mapped_address = (uint32_t)(address & 0x3FFF);
		mapped_address &= mbc->base.rom_size - 1;

		assert(mapped_address < mbc->base.rom_size && "Out of bound ROM");
		*val = memory[mapped_address];

		return 1;
	}
	else if (address >= 0x4000 && address <= 0x7FFF) {
		uint32_t bank = mbc->romb1 << 8 | mbc->romb0;
		uint32_t mapped_address = bank << 14 | (uint32_t)(address & 0x3FFF);
		mapped_address &= mbc->base.rom_size - 1;


		assert(mapped_address < mbc->base.rom_size && "Out of bound ROM");
		*val = memory[mapped_address];

		return 1;
	}
	else if (address >= 0xA000 && address <= 0xBFFF) {
		if (mbc->ramg && mbc->base.ram_size != 0) {
			uint32_t bank = mbc->ramb;
			uint32_t mapped_address = bank << 13 | (uint32_t)(address & 0x1FFF);
			mapped_address &= mbc->base.ram_size - 1;

			*val = memory[mapped_address];
			return 1;
		}
		else {
			return 2;
		}
	}

	return 0;
}

uint8_t gbz80_mbc_005_write(void* mbc_005, uint16_t address, uint8_t val, uint32_t* ram_address)
{
	gbz80_mbc_005_t* mbc = (gbz80_mbc_005_t*)mbc_005;

	if (address >= 0x0000 && address <= 0x1FFF) {
		mbc->ramg = (val & 0x0F) == 0x0A;
		return 1;
	}
	else if (address >= 0x2000 && address <= 0x2FFF) {
		mbc->romb0 = val;
		return 1;
	}
	else if (address >= 0x3000 && address <= 0x3FFF) {
		mbc->romb1 = val & 0x1;
		return 1;
	}
	else if (address >= 0x4000 && address <= 0x5FFF) {
		mbc->ramb = val & 0xF;
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

void gbz80_mbc_005_clock(void* mbc_005)
{
}
