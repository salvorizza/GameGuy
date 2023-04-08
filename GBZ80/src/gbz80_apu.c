#include "gbz80_apu.h"

#include "gbz80.h"

void frame_sequencer_calc_clock(uint8_t num_step, uint8_t* length_counter_clock, uint8_t* sweep_clock, uint8_t* volume_envelope_clock) {
	num_step %= 8;

	*length_counter_clock = num_step % 2 == 0;
	*sweep_clock = num_step == 2 || num_step == 6;
	*volume_envelope_clock = num_step == 7;
}

void gbz80_apu_init(gbz80_apu_t* apu, gbz80_t* instance){
	apu->instance = instance;
	gbz80_apu_frame_sequencer_init(&apu->frame_sequencer);
	apu->so_1 = apu->so_2 = 0;
	gbz80_apu_channel_base_init(&apu->channel_1.base);
	gbz80_apu_channel_base_init(&apu->channel_2.base);
	gbz80_apu_channel_base_init(&apu->channel_3.base);
	gbz80_apu_channel_base_init(&apu->channel_4.base);

	gbz80_apu_duty_init(&apu->channel_1.duty_cycler);
	gbz80_apu_frequency_sweep_init(&apu->channel_1.sweeper, 0, 8, 0, 0);
	apu->channel_1.sweeper.enabled = 0;

	gbz80_apu_duty_init(&apu->channel_2.duty_cycler);
	gbz80_apu_wave_init(&apu->channel_3.wave_cycler);
}

void gbz80_apu_clock(gbz80_apu_t* apu){
	apu->sample_ready = 0;
	uint8_t nr50 = gbz80_memory_read_internal(apu->instance, NR50);
	uint8_t nr51 = gbz80_memory_read_internal(apu->instance, NR51);
	uint8_t nr52 = gbz80_memory_read_internal(apu->instance, NR52);

	uint8_t apu_power= common_get8_bit(nr52, 7);

	if (apu_power) {
		gbz80_apu_frame_sequencer_update(apu, &apu->frame_sequencer);

		if (apu->frame_sequencer.sweep_clock && apu->channel_1.sweeper.enabled) {
			uint8_t nr10 = gbz80_memory_read_internal(apu->instance, NRx0(1));

			uint8_t decrease = common_get8_bit(nr10, 3);
			uint8_t sweep_shift = common_get8_bit_range(nr10, 0, 2);
			uint8_t sweep_state = gbz80_apu_frequency_sweep_update(&apu->channel_1.sweeper,decrease,sweep_shift);

			if (sweep_state && sweep_shift > 0) {
				uint8_t nr14 = gbz80_memory_read_internal(apu->instance, NRx4(1));
				common_change8_bit_range(&nr14, 0, 2, (apu->channel_1.sweeper.frequency_shadow >> 8) & 0x7);
				gbz80_memory_write_internal(apu->instance, NRx3(1), (uint8_t)apu->channel_1.sweeper.frequency_shadow);
				gbz80_memory_write_internal(apu->instance, NRx4(1), nr14);
				apu->channel_1.base.frequency_timer.period = apu->channel_1.sweeper.frequency_shadow;
			} else if (sweep_state == 0) {
				apu->channel_1.base.length_counter.enabled = 0;
				common_change8_bit(&nr52, 0, 0);
				gbz80_memory_write_internal(apu->instance, NR52, nr52);
			}
		}
		else if (apu->frame_sequencer.length_counter_clock) {
			uint8_t nr11 = gbz80_memory_read_internal(apu->instance, NRx1(1));
			uint8_t nr21 = gbz80_memory_read_internal(apu->instance, NRx1(2));
			uint8_t nr31 = gbz80_memory_read_internal(apu->instance, NRx1(3));
			uint8_t nr41 = gbz80_memory_read_internal(apu->instance, NRx1(4));

			uint8_t nr14 = gbz80_memory_read_internal(apu->instance, NRx4(1));
			uint8_t nr24 = gbz80_memory_read_internal(apu->instance, NRx4(2));
			uint8_t nr34 = gbz80_memory_read_internal(apu->instance, NRx4(3));
			uint8_t nr44 = gbz80_memory_read_internal(apu->instance, NRx4(4));

			gbz80_apu_length_counter_update(
				&apu->channel_1.base.length_counter,
				common_get8_bit(nr14, 6)
			);

			gbz80_apu_length_counter_update(
				&apu->channel_2.base.length_counter,
				common_get8_bit(nr24, 6)
			);

			gbz80_apu_length_counter_update(
				&apu->channel_3.base.length_counter,
				common_get8_bit(nr34, 6)
			);

			gbz80_apu_length_counter_update(
				&apu->channel_4.base.length_counter,
				common_get8_bit(nr44, 6)
			);

			

			common_change8_bit(&nr52, 0, apu->channel_1.base.length_counter.enabled);
			common_change8_bit(&nr52, 1, apu->channel_2.base.length_counter.enabled);
			common_change8_bit(&nr52, 2, apu->channel_3.base.length_counter.enabled);
			common_change8_bit(&nr52, 3, apu->channel_4.base.length_counter.enabled);

			if (apu->channel_1.base.length_counter.counter == 0) common_change8_bit_range(&nr11, 0, 5, 0);
			if (apu->channel_2.base.length_counter.counter == 0) common_change8_bit_range(&nr21, 0, 5, 0);
			if (apu->channel_3.base.length_counter.counter == 0) common_change8_bit_range(&nr31, 0, 7, 0);
			if (apu->channel_4.base.length_counter.counter == 0) common_change8_bit_range(&nr41, 0, 5, 0);

			gbz80_memory_write_internal(apu->instance, NRx1(1), nr11);
			gbz80_memory_write_internal(apu->instance, NRx1(2), nr21);
			gbz80_memory_write_internal(apu->instance, NRx1(3), nr31);
			gbz80_memory_write_internal(apu->instance, NRx1(4), nr41);

			gbz80_memory_write_internal(apu->instance, NR52, nr52);
		}
		else if (apu->frame_sequencer.volume_envelope_clock) {
			uint8_t nr12 = gbz80_memory_read_internal(apu->instance, NRx2(1));
			uint8_t nr22 = gbz80_memory_read_internal(apu->instance, NRx2(1));
			uint8_t nr42 = gbz80_memory_read_internal(apu->instance, NRx2(1));

			gbz80_apu_volume_envelope_update(
				&apu->channel_1.base.volume_envelope,
				common_get8_bit(nr12, 3)
			);

			gbz80_apu_volume_envelope_update(
				&apu->channel_2.base.volume_envelope,
				common_get8_bit(nr22, 3)
			);

			gbz80_apu_volume_envelope_update(
				&apu->channel_4.base.volume_envelope,
				common_get8_bit(nr42, 3)
			);
		}

		if (apu->channel_1.base.frequency_timer.period != 0 && apu->channel_1.base.length_counter.enabled == 1 && gbz80_update_timer(&apu->channel_1.base.frequency_timer)) {
			uint8_t nr11 = gbz80_memory_read_internal(apu->instance, NRx1(1));
			uint8_t nr12 = gbz80_memory_read_internal(apu->instance, NRx2(1));
			uint8_t wave_pattern_number = common_get8_bit_range(nr11, 6, 7);

			uint8_t wave_output_duty = gbz80_apu_duty_update(&apu->channel_1.duty_cycler, wave_pattern_number);
			uint8_t wave_output = wave_output_duty * apu->channel_1.base.volume_envelope.counter;

			if (common_get8_bit_range(nr12, 3, 7) != 0)
				apu->channel_1.base.dac_output = wave_output / 7.5 - 1;
			else
				apu->channel_1.base.dac_output = 0;
		}

		if (apu->channel_2.base.frequency_timer.period != 0 && apu->channel_2.base.length_counter.enabled == 1 && gbz80_update_timer(&apu->channel_2.base.frequency_timer)) {
			uint8_t nr21 = gbz80_memory_read_internal(apu->instance, NRx1(2));
			uint8_t nr22 = gbz80_memory_read_internal(apu->instance, NRx2(2));

			uint8_t wave_output = gbz80_apu_duty_update(
				&apu->channel_2.duty_cycler,
				common_get8_bit_range(nr21, 6, 7)
			);

			wave_output *= apu->channel_2.base.volume_envelope.counter;

			if (common_get8_bit_range(nr22, 3, 7) != 0)
				apu->channel_2.base.dac_output = wave_output / 7.5 - 1;
			else
				apu->channel_2.base.dac_output = 0;
		}

		if (apu->channel_3.base.frequency_timer.period != 0 && apu->channel_3.base.length_counter.enabled == 1 && gbz80_update_timer(&apu->channel_3.base.frequency_timer)) {
			uint8_t nr30 = gbz80_memory_read_internal(apu->instance, NRx0(3));
			uint8_t nr31 = gbz80_memory_read_internal(apu->instance, NRx1(3));
			uint8_t nr32 = gbz80_memory_read_internal(apu->instance, NRx2(3));

			uint8_t wave_output = gbz80_apu_wave_update(
				&apu->channel_3.wave_cycler,
				apu,
				common_get8_bit_range(nr32, 5, 6)
			);


			if (common_get8_bit(nr30, 7) != 0)
				apu->channel_3.base.dac_output = wave_output / 7.5 - 1;
			else
				apu->channel_3.base.dac_output = 0;
		}

		if (apu->channel_4.base.frequency_timer.period != 0 && apu->channel_4.base.length_counter.enabled == 1 && gbz80_update_timer(&apu->channel_4.base.frequency_timer)) {
			uint8_t nr41 = gbz80_memory_read_internal(apu->instance, NRx1(4));
			uint8_t nr42 = gbz80_memory_read_internal(apu->instance, NRx2(4));
			uint8_t nr43 = gbz80_memory_read_internal(apu->instance, NRx3(4));

			uint8_t xor_result = common_get8_bit(common_get8_bit((uint8_t)apu->channel_4.lfsr, 0) ^ common_get8_bit((uint8_t)apu->channel_4.lfsr, 1),0);
			uint8_t low_byte = apu->channel_4.lfsr & 0xFF;
			uint8_t high_byte = (apu->channel_4.lfsr >> 8) & 0xFF;

			apu->channel_4.lfsr >>= 1;

			common_change8_bit(&high_byte, 14, xor_result);
			if (common_get8_bit(nr43, 3) == 1)
				common_change8_bit(&low_byte, 6, xor_result);
			
			apu->channel_4.lfsr = low_byte | (high_byte << 8);

			uint8_t wave_output_duty = common_get8_bit(~low_byte, 0);
			uint8_t wave_output = wave_output_duty * apu->channel_4.base.volume_envelope.counter;


			apu->channel_4.base.dac_output = wave_output / 7.5 - 1;
		}
	}

	if (apu->sample_timer.period != 0 && gbz80_update_timer(&apu->sample_timer)) {
		double dac_outs[4] = {
			apu->channel_1.base.dac_enable ? apu->channel_1.base.dac_output : 0,
			apu->channel_2.base.dac_enable ? apu->channel_2.base.dac_output : 0,
			apu->channel_3.base.dac_enable ? apu->channel_3.base.dac_output : 0,
			apu->channel_4.base.dac_enable ? apu->channel_4.base.dac_output : 0,
		};

		apu->so_1 = apu->so_2 = 0.0;
		for (uint8_t channel_number = 0; channel_number < 4; channel_number++) {
			if (common_get8_bit(nr51, channel_number)) {
				apu->so_1 += dac_outs[channel_number];
			}

			if (common_get8_bit(nr51, 4 + channel_number)) {
				apu->so_2 += dac_outs[channel_number];
			}
		}
		apu->so_1 /= 4.0;
		apu->so_2 /= 4.0;

		apu->sample_ready = 1;
	}
}

void gbz80_apu_memory_read(gbz80_apu_t* apu, uint16_t address, uint8_t* readed_val) {
	static uint8_t register_masks[] = {
		0x80,0x3F,0x00,0xFF,0xBF,
		0xFF,0x3F,0x00,0xFF,0xBF,
		0x7F,0xFF,0x9F,0xFF,0xBF,
		0xFF,0xFF,0x00,0x00,0xBF,
		0x00,0x00,0x70
	};

	if (address >= 0xFF10 && address <= 0xFF26) {
		*readed_val |= register_masks[address - 0xFF10];
	} else if (address >= 0xFF27 && address <= 0xFF2F) {
		*readed_val |= 0xFF;
	}
}

uint8_t gbz80_apu_memory_write(gbz80_apu_t* apu, uint16_t address, uint8_t current_value, uint8_t* val) {
	uint8_t nr52 = gbz80_memory_read_internal(apu->instance, NR52);
	
	if (common_get8_bit(nr52, 7) == 0 && address != NR52 && address >= 0xFF10 && address <= 0xFF25) {
		*val = current_value;
		return 1;
	}


	switch (address) {
		// NRx0
		case NRx0(3):
			apu->channel_3.base.dac_enable = common_get8_bit(*val, 7);
			apu->channel_3.base.length_counter.enabled = apu->channel_3.base.dac_enable;
			if (apu->channel_3.base.length_counter.enabled == 0) {
				common_change8_bit(&nr52, 2, apu->channel_3.base.length_counter.enabled);
				gbz80_memory_write_internal(apu->instance, NR52, nr52);
			}
		break;

		//NRx1
		case NRx1(1):
			if (common_get8_bit(nr52, 7) == 1)
				gbz80_apu_length_counter_init(
					&apu->channel_1.base.length_counter,
					64 - common_get8_bit_range(*val, 0, 5),
					apu->channel_1.base.length_counter.enabled
				);
			break;

		case NRx1(2):
			if (common_get8_bit(nr52, 7) == 1)
				gbz80_apu_length_counter_init(
					&apu->channel_2.base.length_counter,
					64 - common_get8_bit_range(*val, 0, 5),
					apu->channel_2.base.length_counter.enabled
				);
			break;

		case NRx1(3):
			if (common_get8_bit(nr52, 7) == 1)
				gbz80_apu_length_counter_init(
					&apu->channel_3.base.length_counter,
					256 - common_get8_bit_range(*val, 0, 7),
					apu->channel_3.base.length_counter.enabled
				);
			break;

		case NRx1(4):
			if (common_get8_bit(nr52, 7) == 1)
				gbz80_apu_length_counter_init(
					&apu->channel_4.base.length_counter,
					64 - common_get8_bit_range(*val, 0, 5),
					apu->channel_4.base.length_counter.enabled
				);
			break;

			//NRx2
		case NRx2(1):
			apu->channel_1.base.dac_enable = (*val & 0xF8) != 0;
			apu->channel_1.base.length_counter.enabled = apu->channel_1.base.dac_enable;
			if (apu->channel_1.base.length_counter.enabled == 0) {
				common_change8_bit(&nr52, 0, apu->channel_1.base.length_counter.enabled);
				gbz80_memory_write_internal(apu->instance, NR52, nr52);
			}
			break;

		case NRx2(2):
			apu->channel_2.base.dac_enable = (*val & 0xF8) != 0;
			apu->channel_2.base.length_counter.enabled = apu->channel_2.base.dac_enable;
			if (apu->channel_2.base.length_counter.enabled == 0) {
				common_change8_bit(&nr52, 1, apu->channel_2.base.length_counter.enabled);
				gbz80_memory_write_internal(apu->instance, NR52, nr52);
			}
			break;

		case NRx2(3):
			//YOu cannot disable DAC using NR32
			break;

		case NRx2(4):
			apu->channel_4.base.dac_enable = (*val & 0xF8) != 0;
			apu->channel_4.base.length_counter.enabled = apu->channel_4.base.dac_enable;
			if (apu->channel_4.base.length_counter.enabled == 0) {
				common_change8_bit(&nr52, 3, apu->channel_4.base.length_counter.enabled);
				gbz80_memory_write_internal(apu->instance, NR52, nr52);
			}
			break;

		//NRx4
		case NRx4(1): {
			uint8_t length_counter_clock, sweep_clock, volume_envelope_clock;
			frame_sequencer_calc_clock(apu->frame_sequencer.num_step, &length_counter_clock, &sweep_clock, &volume_envelope_clock);
			if (length_counter_clock == 0 && apu->channel_1.base.length_counter.enabled && apu->channel_1.base.length_counter.counter != 0) {
				apu->channel_1.base.length_counter.counter -= 1;

				if (apu->channel_1.base.length_counter.counter == 0) {
					apu->channel_1.base.length_counter.enabled = 0;
					common_change8_bit(&nr52, 0, 0);
					gbz80_memory_write_internal(apu->instance, NR52, nr52);
				}
			}

			gbz80_memory_write_internal(apu->instance, address, *val);
			if (common_get8_bit(*val, 7) && apu->channel_1.base.dac_enable)
				gbz80_apu_trigger_channel1(apu);
			break;
		}

		case NRx4(2):
			gbz80_memory_write_internal(apu->instance, address, *val);
			if (common_get8_bit(*val, 7) && apu->channel_2.base.dac_enable)
				gbz80_apu_trigger_channel2(apu);
			break;

		case NRx4(3):
			gbz80_memory_write_internal(apu->instance, address, *val);
			if (common_get8_bit(*val, 7) && apu->channel_3.base.dac_enable)
				gbz80_apu_trigger_channel3(apu);
			break;

		case NRx4(4):
			gbz80_memory_write_internal(apu->instance, address, *val);
			if (common_get8_bit(*val, 7) && apu->channel_4.base.dac_enable)
				gbz80_apu_trigger_channel4(apu);
				break;

		case NR52: {
			uint8_t previous = common_get8_bit_range(current_value, 0, 6);
			common_change8_bit_range(val, 0, 6, previous);

			if (common_get8_bit(*val, 7) == 0) {
				for (uint16_t base_address = 0xFF10; base_address <= 0xFF25; base_address++) {
					gbz80_memory_write_internal(apu->instance, base_address, 0x00);
				}

				*val = 0x70;
			} else {
				apu->frame_sequencer.num_step = 0;
				apu->channel_1.duty_cycler.wave_position = 0;
				apu->channel_2.duty_cycler.wave_position = 0;
				apu->channel_3.wave_cycler.byte_position = 0;
				apu->channel_3.wave_cycler.nibble_position = 0;
			}
			break;
		}
	}

	return 1;
}

void gbz80_apu_trigger_channel1(gbz80_apu_t* apu) {
	uint8_t nr10 = gbz80_memory_read_internal(apu->instance, NRx0(1));
	uint8_t nr11 = gbz80_memory_read_internal(apu->instance, NRx1(1));
	uint8_t nr12 = gbz80_memory_read_internal(apu->instance, NRx2(1));
	uint8_t nr13 = gbz80_memory_read_internal(apu->instance, NRx3(1));
	uint8_t nr14 = gbz80_memory_read_internal(apu->instance, NRx4(1));
	uint8_t nr52 = gbz80_memory_read_internal(apu->instance, NR52);

	uint16_t frequency = nr13 | ((uint16_t)common_get8_bit_range(nr14, 0, 2) << 8);
	uint8_t sweep_time = common_get8_bit_range(nr10, 4, 6);
	uint8_t sweep_decrease = common_get8_bit(nr10, 3);
	uint8_t sweep_shift = common_get8_bit_range(nr10, 0, 2);
	uint8_t volume = common_get8_bit_range(nr12, 4, 7);
	uint8_t volume_period = common_get8_bit_range(nr12, 0, 2);
	uint16_t length_counter_period = apu->channel_1.base.length_counter.counter == 0 ? 64 - common_get8_bit_range(nr11, 0, 5) : apu->channel_1.base.length_counter.counter;
	uint8_t channel_enabled = gbz80_apu_frequency_sweep_init(&apu->channel_1.sweeper, frequency, sweep_time, sweep_decrease, sweep_shift);

	uint8_t length_counter_clock, sweep_clock, volume_envelope_clock;
	frame_sequencer_calc_clock(apu->frame_sequencer.num_step, &length_counter_clock, &sweep_clock, &volume_envelope_clock);

	if (length_counter_clock == 0 && channel_enabled && length_counter_period == 64 && apu->channel_1.base.length_counter.counter == 0) {
		length_counter_period = 63;
	}

	if (volume_envelope_clock) {
		volume_period += 1;
	}

	gbz80_apu_length_counter_init(&apu->channel_1.base.length_counter, length_counter_period, channel_enabled);
	gbz80_init_timer(&apu->channel_1.base.frequency_timer, (2048llu - frequency) * 4);
	gbz80_apu_volume_envelope_init(&apu->channel_1.base.volume_envelope, volume, volume_period);

	common_change8_bit(&nr52, 0, channel_enabled);

	gbz80_memory_write_internal(apu->instance, NR52, nr52);
}

void gbz80_apu_trigger_channel2(gbz80_apu_t* apu) {
	uint8_t nr21 = gbz80_memory_read_internal(apu->instance, NRx1(2));
	uint8_t nr22 = gbz80_memory_read_internal(apu->instance, NRx2(2));
	uint8_t nr23 = gbz80_memory_read_internal(apu->instance, NRx3(2));
	uint8_t nr24 = gbz80_memory_read_internal(apu->instance, NRx4(2));
	uint8_t nr52 = gbz80_memory_read_internal(apu->instance, NR52);
	uint8_t channel_enabled = 1;

	uint16_t frequency = nr23 | ((uint16_t)common_get8_bit_range(nr24, 0, 2) << 8);
	uint8_t volume = common_get8_bit_range(nr22, 4, 7);
	uint8_t volume_period = common_get8_bit_range(nr22, 0, 2);
	uint16_t length_counter_period = apu->channel_2.base.length_counter.counter == 0 ? 64 - common_get8_bit_range(nr21, 0, 5) : apu->channel_2.base.length_counter.counter;

	gbz80_apu_length_counter_init(&apu->channel_2.base.length_counter, length_counter_period, channel_enabled);
	gbz80_init_timer(&apu->channel_2.base.frequency_timer, (2048llu - frequency) * 4);
	gbz80_apu_volume_envelope_init(&apu->channel_2.base.volume_envelope, volume, volume_period);

	common_change8_bit(&nr52, 1, channel_enabled);

	gbz80_memory_write_internal(apu->instance, NR52, nr52);
}

void gbz80_apu_trigger_channel3(gbz80_apu_t* apu) {
	uint8_t nr31 = gbz80_memory_read_internal(apu->instance, NRx1(3));
	uint8_t nr32 = gbz80_memory_read_internal(apu->instance, NRx2(3));
	uint8_t nr33 = gbz80_memory_read_internal(apu->instance, NRx3(3));
	uint8_t nr34 = gbz80_memory_read_internal(apu->instance, NRx4(3));
	uint8_t nr52 = gbz80_memory_read_internal(apu->instance, NR52);
	uint8_t channel_enabled = 1;

	uint16_t frequency = nr33 | ((uint16_t)common_get8_bit_range(nr34, 0, 2) << 8);
	uint16_t length_counter_period = apu->channel_3.base.length_counter.counter == 0 ? 256 - common_get8_bit_range(nr31, 0, 7) : apu->channel_3.base.length_counter.counter;

	gbz80_apu_length_counter_init(&apu->channel_3.base.length_counter, length_counter_period, channel_enabled);
	gbz80_init_timer(&apu->channel_3.base.frequency_timer, (2048llu - frequency) * 2);
	gbz80_apu_wave_init(&apu->channel_3.wave_cycler);

	common_change8_bit(&nr52, 2, channel_enabled);

	gbz80_memory_write_internal(apu->instance, NR52, nr52);
}

void gbz80_apu_trigger_channel4(gbz80_apu_t* apu) {
	uint8_t nr41 = gbz80_memory_read_internal(apu->instance, NRx1(4));
	uint8_t nr42 = gbz80_memory_read_internal(apu->instance, NRx2(4));
	uint8_t nr43 = gbz80_memory_read_internal(apu->instance, NRx3(4));
	uint8_t nr44 = gbz80_memory_read_internal(apu->instance, NRx4(4));
	uint8_t nr52 = gbz80_memory_read_internal(apu->instance, NR52);
	uint8_t channel_enabled = 1;

	uint8_t s = common_get8_bit_range(nr43, 4, 7);
	uint8_t r = common_get8_bit_range(nr43, 0, 2);

	uint8_t frequency = 0;
	switch (r) {
		case 0: frequency = 8; break;
		case 1: frequency = 16; break;
		case 2: frequency = 32; break;
		case 3: frequency = 48; break;
		case 4: frequency = 64; break;
		case 5: frequency = 80; break;
		case 6: frequency = 96; break;
		case 7: frequency = 112; break;
	}

	uint8_t volume = common_get8_bit_range(nr42, 4, 7);
	uint8_t volume_period = common_get8_bit_range(nr42, 0, 2);
	uint16_t length_counter_period = apu->channel_4.base.length_counter.counter == 0 ? 64 - common_get8_bit_range(nr41, 0, 5) : apu->channel_4.base.length_counter.counter;


	gbz80_apu_length_counter_init(&apu->channel_4.base.length_counter, length_counter_period, channel_enabled);
	apu->channel_4.lfsr = 0xFF;
	gbz80_init_timer(&apu->channel_4.base.frequency_timer, frequency);
	gbz80_apu_volume_envelope_init(&apu->channel_4.base.volume_envelope, volume, volume_period);

	common_change8_bit(&nr52, 3, channel_enabled);

	gbz80_memory_write_internal(apu->instance, NR52, nr52);
}

void gbz80_apu_channel_base_init(gbz80_apu_base_channel_t* channel) {
	channel->dac_output = 0.0;
	channel->dac_enable = 0;
	gbz80_init_timer(&channel->frequency_timer, 0);
	gbz80_apu_length_counter_init(&channel->length_counter, 0, 0);
	gbz80_apu_volume_envelope_init(&channel->volume_envelope, 0, 0);
}

void gbz80_apu_frame_sequencer_init(gbz80_apu_frame_sequencer_t* frame_sequencer) {
	frame_sequencer->length_counter_clock = 0;
	frame_sequencer->sweep_clock = 0;
	frame_sequencer->volume_envelope_clock = 0;
	frame_sequencer->num_step = 0;
	frame_sequencer->div_last = 0;
}

void gbz80_apu_frame_sequencer_update(gbz80_apu_t* apu, gbz80_apu_frame_sequencer_t* frame_sequencer) {
	uint8_t div = 0;

	gbz80_cpu_memory_read(&apu->instance->cpu, 0xFF04, &div);
	uint8_t div_current = common_get8_bit(div, 3);
	uint8_t fs_update = frame_sequencer->div_last == 1 && div_current == 0;
	frame_sequencer->div_last = div_current;

	if (fs_update) {
		frame_sequencer_calc_clock(frame_sequencer->num_step,
									&frame_sequencer->length_counter_clock,
									&frame_sequencer->sweep_clock,
									&frame_sequencer->volume_envelope_clock
		);
	
		frame_sequencer->num_step++;
		frame_sequencer->num_step %= 8;
	}
	else {
		frame_sequencer->length_counter_clock = 0;
		frame_sequencer->sweep_clock = 0;
		frame_sequencer->volume_envelope_clock = 0;

	}
}



void gbz80_apu_length_counter_init(gbz80_apu_length_counter_t* length_counter, uint16_t counter, uint8_t enabled)
{
	length_counter->counter = counter;
	length_counter->enabled = enabled;
}

void gbz80_apu_length_counter_update(gbz80_apu_length_counter_t* length_counter, uint8_t length_enable)
{
	if (length_enable && length_counter->counter != 0) {
		length_counter->counter--;
		if (length_counter->counter == 0) {
			length_counter->enabled = 0;
		}
	}
}

void gbz80_apu_volume_envelope_init(gbz80_apu_volume_envelope_t* volume_envelope, uint8_t volume, uint8_t period)
{
	volume_envelope->counter = volume;
	gbz80_init_timer(&volume_envelope->timer, period == 0 ? 8 : period);
}

void gbz80_apu_volume_envelope_update(gbz80_apu_volume_envelope_t* volume_envelope, uint8_t increment)
{
	if (gbz80_update_timer(&volume_envelope->timer) && volume_envelope->timer.period != 0) {
		if ((volume_envelope->counter < 0xF && increment) || (volume_envelope->counter > 0x0 && !increment)) {
			uint8_t adjustment = 0;
			if(increment) {
				adjustment = +1;
			} else {
				adjustment = -1;
			}
			volume_envelope->counter += adjustment;
		}
	}
}

uint16_t gbz80_apu_frequency_sweep_calc(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency, uint8_t decrease, uint8_t sweep_shift, uint8_t* channel_enabled) {
	uint16_t new_frequency = frequency_sweep->frequency_shadow >> sweep_shift;

	if (decrease) {
		new_frequency = frequency_sweep->frequency_shadow - new_frequency;
	} else {
		new_frequency = frequency_sweep->frequency_shadow + new_frequency;
	}

	if (new_frequency > 2047) {
		*channel_enabled = 0;
	} else {
		*channel_enabled = 1;
	}

	return new_frequency;
}

uint8_t gbz80_apu_frequency_sweep_init(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency, uint8_t sweep_time, uint8_t decrease, uint8_t sweep_shift) {
	uint8_t channel_enabled = 1;

	sweep_time = sweep_time == 0 ? 8 : sweep_time;

	frequency_sweep->frequency_shadow = frequency;
	gbz80_init_timer(&frequency_sweep->timer, sweep_time);
	frequency_sweep->enabled = sweep_time != 0 || sweep_shift != 0;

	if (sweep_shift != 0) {
		uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift, &channel_enabled);
	}

	return channel_enabled;
}

uint8_t gbz80_apu_frequency_sweep_update(gbz80_apu_frequency_sweep_t* frequency_sweep, uint8_t decrease, uint8_t sweep_shift) {
	uint8_t channel_enabled = 2,th;
	if (gbz80_update_timer(&frequency_sweep->timer)) {
		if (frequency_sweep->timer.period > 0) {
			frequency_sweep->timer.counter = frequency_sweep->timer.period;
		} else {
			frequency_sweep->timer.counter = 8;
		}

		if (frequency_sweep->enabled && frequency_sweep->timer.period > 0) {
			uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift, &th);

			if (th && sweep_shift > 0) {
				frequency_sweep->frequency_shadow = new_frequency;

				uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift, &channel_enabled);
			}
		}

		return channel_enabled;
	}
	
	return channel_enabled;
}

void gbz80_apu_duty_init(gbz80_apu_duty_cycle_t* duty_cycler) {
	duty_cycler->wave_position = 0;
}

uint8_t gbz80_apu_duty_update(gbz80_apu_duty_cycle_t* duty_cycler, uint8_t wave_pattern_number) {
	static uint8_t waves[4] = {
		0b11111110,
		0b01111110,
		0b01111000,
		0b10000000
	};

	uint8_t wave_output = common_get8_bit(waves[wave_pattern_number], duty_cycler->wave_position);
	
	duty_cycler->wave_position++;
	duty_cycler->wave_position %= 8;

	return wave_output;
}

void gbz80_apu_wave_init(gbz80_apu_wave_cycle_t* wave_cycle)
{
	wave_cycle->byte_position = 0;
	wave_cycle->nibble_position = 0;
}

uint8_t gbz80_apu_wave_update(gbz80_apu_wave_cycle_t* wave_cycle, gbz80_apu_t* apu, uint8_t volume_code)
{
	wave_cycle->nibble_position++;
	if (wave_cycle->nibble_position > 1) {
		wave_cycle->byte_position++;
		if (wave_cycle->byte_position > 0xF) {
			wave_cycle->byte_position = 0;
		}
		wave_cycle->nibble_position = 0;
	}

	uint8_t two_samples = gbz80_memory_read_internal(apu->instance, WAVE_BASE + wave_cycle->byte_position);
	uint8_t bit_start = 4 - (wave_cycle->nibble_position * 4);
	uint8_t sample = common_get8_bit_range(two_samples, bit_start, bit_start + 3);

	switch (volume_code) {
		case 0: sample >>= 4; break;
		case 1: sample >>= 0; break;
		case 2: sample >>= 1; break;
		case 3: sample >>= 2; break;
	}

	return sample;
}
