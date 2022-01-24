#include "gbz80_apu.h"

#include "gbz80.h"

void gbz80_apu_init(gbz80_apu_t* apu, gbz80_t* instance){
	apu->instance = instance;
	gbz80_apu_frame_sequencer_init(&apu->frame_sequencer);
	apu->so_1 = apu->so_2 = 0;
	gbz80_apu_channel_base_init(&apu->channel_1.base);
	gbz80_apu_channel_base_init(&apu->channel_2.base);
	gbz80_apu_channel_base_init(&apu->channel_3.base);

	apu->channel_1.base.dac_output = 0;
	apu->channel_2.base.dac_output = 0;
	apu->channel_3.base.dac_output = 0;

	gbz80_apu_duty_init(&apu->channel_1.duty_cycler);
	gbz80_apu_frequency_sweep_init(&apu->channel_1.sweeper, 0, 8, 0, 0);

	gbz80_apu_duty_init(&apu->channel_2.duty_cycler);

	gbz80_apu_wave_init(&apu->channel_3.wave_cycler);
}

void gbz80_apu_step(gbz80_apu_t* apu, size_t num_cycles){
	apu->status = 0;
	uint8_t nr50 = gbz80_memory_read8(apu->instance, 0xFF20);
	uint8_t nr51 = gbz80_memory_read8(apu->instance, 0xFF25);
	uint8_t nr52 = gbz80_memory_read8(apu->instance, 0xFF26);

	uint8_t apu_power= common_get8_bit(nr52, 7);

	uint8_t left_channels_power[4] = {
		common_get8_bit(nr51, 0),
		common_get8_bit(nr51, 1),
		common_get8_bit(nr51, 2),
		common_get8_bit(nr51, 3)
	};

	uint8_t right_channels_power[4] = {
		common_get8_bit(nr51, 4),
		common_get8_bit(nr51, 5),
		common_get8_bit(nr51, 6),
		common_get8_bit(nr51, 7)
	};

	for (size_t i = 0; i < num_cycles; i++) {
		if (apu_power) {
			gbz80_apu_frame_sequencer_update(&apu->frame_sequencer);


			if (apu->frame_sequencer.sweep_clock) {
				uint8_t nr10 = gbz80_memory_read8(apu->instance, 0xFF10);

				uint8_t sweep_state = gbz80_apu_frequency_sweep_update(
					&apu->channel_1.sweeper,
					common_get8_bit(nr10, 3),
					common_get8_bit_range(nr10, 0, 2)
				);

				if (sweep_state == 1) {
					uint8_t nr14 = gbz80_memory_read8(apu->instance, 0xFF14);
					common_change8_bit_range(&nr14, 0, 2, (apu->channel_1.sweeper.frequency_shadow >> 8) & 0x7);

					gbz80_memory_write8(apu->instance, 0xFF13, (uint8_t)apu->channel_1.sweeper.frequency_shadow);
					gbz80_memory_write8(apu->instance, 0xFF14, nr14);

					apu->channel_1.base.frequency_timer.period = apu->channel_1.sweeper.frequency_shadow;
				}
			}
			else if (apu->frame_sequencer.length_counter_clock) {
				uint8_t nr14 = gbz80_memory_read8(apu->instance, 0xFF14);
				uint8_t nr24 = gbz80_memory_read8(apu->instance, 0xFF19);
				uint8_t nr34 = gbz80_memory_read8(apu->instance, 0xFF1E);

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
			}
			else if (apu->frame_sequencer.volume_envelope_clock) {
				uint8_t nr12 = gbz80_memory_read8(apu->instance, 0xFF12);
				uint8_t nr22 = gbz80_memory_read8(apu->instance, 0xFF17);

				gbz80_apu_volume_envelope_update(
					&apu->channel_1.base.volume_envelope,
					common_get8_bit(nr12, 3)
				);

				gbz80_apu_volume_envelope_update(
					&apu->channel_2.base.volume_envelope,
					common_get8_bit(nr22, 3)
				);
			}

			if (apu->channel_1.base.frequency_timer.period != 0 && apu->channel_1.base.length_counter.enabled == 1 && gbz80_apu_update_timer(&apu->channel_1.base.frequency_timer)) {
				uint8_t nr11 = gbz80_memory_read8(apu->instance, 0xFF11);
				uint8_t nr12 = gbz80_memory_read8(apu->instance, 0xFF12);

				uint8_t wave_output_duty = gbz80_apu_duty_update(
					&apu->channel_1.duty_cycler,
					common_get8_bit_range(nr11, 6, 7)
				);

				uint8_t wave_output = wave_output_duty * apu->channel_1.base.volume_envelope.counter;

				if (common_get8_bit_range(nr12, 3, 7) != 0)
					apu->channel_1.base.dac_output = wave_output / 7.5 - 1;
				else
					apu->channel_1.base.dac_output = 0;
			}

			if (apu->channel_2.base.frequency_timer.period != 0 && apu->channel_2.base.length_counter.enabled == 1 && gbz80_apu_update_timer(&apu->channel_2.base.frequency_timer)) {
				uint8_t nr21 = gbz80_memory_read8(apu->instance, 0xFF16);
				uint8_t nr22 = gbz80_memory_read8(apu->instance, 0xFF17);

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

			if (apu->channel_3.base.frequency_timer.period != 0 && apu->channel_3.base.length_counter.enabled == 1 && gbz80_apu_update_timer(&apu->channel_3.base.frequency_timer)) {
				uint8_t nr30 = gbz80_memory_read8(apu->instance, 0xFF1A);
				uint8_t nr31 = gbz80_memory_read8(apu->instance, 0xFF1B);
				uint8_t nr32 = gbz80_memory_read8(apu->instance, 0xFF1C);

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
			//gbz80_apu_channel_1_clock(apu);
			//gbz80_apu_channel_2_clock(apu);
			//gbz80_apu_channel_3_clock(apu);
			//gbz80_apu_channel_4_clock(apu);

			

			if (apu->sample_function != NULL) {
				if (apu->sample_timer.period != 0 && gbz80_apu_update_timer(&apu->sample_timer)) {
					double dac_outs[4] = {
						apu->channel_1.base.dac_output,
						apu->channel_2.base.dac_output,
						apu->channel_3.base.dac_output,
						0
					};

					apu->so_1 = apu->so_2 = 0.0;
					for (uint8_t channel_number = 0; channel_number < 4; channel_number++) {
						if (left_channels_power[channel_number]) {
							apu->so_1 += dac_outs[channel_number];
						}

						if (right_channels_power[channel_number]) {
							apu->so_2 += dac_outs[channel_number];
						}
					}
					apu->so_1 /= 4.0;
					apu->so_2 /= 4.0;

					apu->status = 1;
				}
			}
			
		}
	}
}

uint8_t gbz80_apu_memory_read(gbz80_apu_t* apu, uint16_t address) {
	return 1;
}

uint8_t gbz80_apu_memory_write(gbz80_apu_t* apu, uint16_t address, uint8_t val) {
	switch (address) {
		//NRx1
		case 0xFF11:
			gbz80_apu_length_counter_init(
				&apu->channel_1.base.length_counter,
				64 - common_get8_bit_range(val, 0, 5),
				apu->channel_1.base.length_counter.enabled
			);
			break;

		case 0xFF16:
			gbz80_apu_length_counter_init(
				&apu->channel_2.base.length_counter,
				64 - common_get8_bit_range(val, 0, 5),
				apu->channel_2.base.length_counter.enabled
			);
			break;

		case 0xFF1B:
			gbz80_apu_length_counter_init(
				&apu->channel_3.base.length_counter,
				256 - common_get8_bit_range(val, 0, 5),
				apu->channel_3.base.length_counter.enabled
			);
			break;

		case 0xFF20:
			//TODO
			break;

		//NRx4
		case 0xFF14:
			if (common_get8_bit(val, 7))
				gbz80_apu_trigger_channel1(apu);
			break;

		case 0xFF19:
			if (common_get8_bit(val, 7))
				gbz80_apu_trigger_channel2(apu);
			break;

		case 0xFF1E:
			if (common_get8_bit(val, 7))
				gbz80_apu_trigger_channel3(apu);
			break;

		case 0xFF23:
			if (common_get8_bit(val, 7))
				gbz80_apu_trigger_channel4(apu);
			break;
	}

	return 1;
}

void gbz80_apu_trigger_channel1(gbz80_apu_t* apu) {
	uint8_t nr10 = gbz80_memory_read8(apu->instance, 0xFF10);
	uint8_t nr12 = gbz80_memory_read8(apu->instance, 0xFF12);
	uint8_t nr13 = gbz80_memory_read8(apu->instance, 0xFF13);
	uint8_t nr14 = gbz80_memory_read8(apu->instance, 0xFF14);
	uint16_t frequency = nr13 | ((uint16_t)common_get8_bit_range(nr14, 0, 2) << 8);
	uint8_t sweep_time = common_get8_bit_range(nr10, 4, 6);

	gbz80_apu_length_counter_init(
		&apu->channel_1.base.length_counter, 
		apu->channel_1.base.length_counter.counter == 0 ? 64 : apu->channel_1.base.length_counter.counter, 
		1
	);	
	gbz80_apu_init_timer(&apu->channel_1.base.frequency_timer, (2048 - frequency) * 4);
	gbz80_apu_volume_envelope_init(
		&apu->channel_1.base.volume_envelope,
		common_get8_bit_range(nr12, 4, 7),
		common_get8_bit_range(nr12, 0, 2)
	);
	uint8_t channel_enabled = gbz80_apu_frequency_sweep_init(
		&apu->channel_1.sweeper,
		frequency,
		sweep_time == 0 ? 8 : sweep_time,
		common_get8_bit(nr10, 3),
		common_get8_bit_range(nr10, 0, 2)
	);
	gbz80_apu_duty_init(&apu->channel_1.duty_cycler);
	apu->channel_1.base.length_counter.enabled = channel_enabled;
	//gbz80_apu_frame_sequencer_init(&apu->frame_sequencer);

}

void gbz80_apu_trigger_channel2(gbz80_apu_t* apu)
{
	uint8_t nr22 = gbz80_memory_read8(apu->instance, 0xFF17);
	uint8_t nr23 = gbz80_memory_read8(apu->instance, 0xFF18);
	uint8_t nr24 = gbz80_memory_read8(apu->instance, 0xFF19);
	uint16_t frequency = nr23 | ((uint16_t)common_get8_bit_range(nr24, 0, 2) << 8);

	gbz80_apu_length_counter_init(
		&apu->channel_2.base.length_counter,
		apu->channel_2.base.length_counter.counter == 0 ? 64 : apu->channel_1.base.length_counter.counter,
		1
	);
	gbz80_apu_init_timer(&apu->channel_2.base.frequency_timer, (2048 - frequency) * 4);
	gbz80_apu_volume_envelope_init(
		&apu->channel_2.base.volume_envelope,
		common_get8_bit_range(nr22, 4, 7),
		common_get8_bit_range(nr22, 0, 2)
	);
	gbz80_apu_duty_init(&apu->channel_2.duty_cycler);
}

void gbz80_apu_trigger_channel3(gbz80_apu_t* apu)
{
	uint8_t nr32 = gbz80_memory_read8(apu->instance, 0xFF1C);
	uint8_t nr33 = gbz80_memory_read8(apu->instance, 0xFF1D);
	uint8_t nr34 = gbz80_memory_read8(apu->instance, 0xFF1E);
	uint16_t frequency = nr33 | ((uint16_t)common_get8_bit_range(nr34, 0, 2) << 8);

	gbz80_apu_length_counter_init(
		&apu->channel_3.base.length_counter,
		apu->channel_3.base.length_counter.counter == 0 ? 256 : apu->channel_1.base.length_counter.counter,
		1
	);
	gbz80_apu_init_timer(&apu->channel_3.base.frequency_timer, (2048 - frequency) * 2);
	gbz80_apu_wave_init(&apu->channel_3.wave_cycler);
}

void gbz80_apu_trigger_channel4(gbz80_apu_t* apu)
{
}

void gbz80_apu_channel_base_init(gbz80_apu_base_channel_t* channel) {
	channel->dac_output = 0.0;
	gbz80_apu_init_timer(&channel->frequency_timer, 0);
	gbz80_apu_length_counter_init(&channel->length_counter, 0, 0);
	gbz80_apu_volume_envelope_init(&channel->volume_envelope, 0, 0);
}

void gbz80_apu_frame_sequencer_init(gbz80_apu_frame_sequencer_t* frame_sequencer) {
	frame_sequencer->length_counter_clock = 0;
	frame_sequencer->sweep_clock = 0;
	frame_sequencer->volume_envelope_clock = 0;
	frame_sequencer->num_step = 0;
	gbz80_apu_init_timer(&frame_sequencer->timer, GBZ80_APU_FREQ / 512);
}

void gbz80_apu_frame_sequencer_update(gbz80_apu_frame_sequencer_t* frame_sequencer) {
	if (gbz80_apu_update_timer(&frame_sequencer->timer)) {
		frame_sequencer->length_counter_clock = frame_sequencer->num_step % 2 == 0;
		frame_sequencer->sweep_clock = frame_sequencer->num_step == 2 || frame_sequencer->num_step == 6;
		frame_sequencer->volume_envelope_clock = frame_sequencer->num_step % 8 == 7;
	
		frame_sequencer->num_step++;
		frame_sequencer->num_step %= 8;
	}
	else {
		frame_sequencer->length_counter_clock = 0;
		frame_sequencer->sweep_clock = 0;
		frame_sequencer->volume_envelope_clock = 0;

	}
}

void gbz80_apu_length_counter_init(gbz80_apu_length_counter_t* length_counter, uint8_t counter, uint8_t enabled)
{
	length_counter->counter = counter;
	length_counter->enabled = enabled;
}

void gbz80_apu_length_counter_update(gbz80_apu_length_counter_t* length_counter, uint8_t length_enable)
{
	if (length_enable && length_counter != 0) {
		length_counter->counter--;
		if (length_counter == 0) {
			length_counter->enabled = 0;
		}
	}
}

void gbz80_apu_volume_envelope_init(gbz80_apu_volume_envelope_t* volume_envelope, uint8_t volume, uint8_t period)
{
	volume_envelope->counter = volume;
	gbz80_apu_init_timer(&volume_envelope->timer, period);
}

void gbz80_apu_volume_envelope_update(gbz80_apu_volume_envelope_t* volume_envelope, uint8_t increment)
{
	if (volume_envelope->timer.period != 0 && gbz80_apu_update_timer(&volume_envelope->timer)) {
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
	}
	else {
		new_frequency = frequency_sweep->frequency_shadow + new_frequency;
	}

	if(new_frequency > 2047) {
		*channel_enabled = 0;
	}
	else {
		*channel_enabled = 1;
	}

	return new_frequency;
}

uint8_t gbz80_apu_frequency_sweep_init(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency, uint8_t sweep_time, uint8_t decrease, uint8_t sweep_shift) {
	frequency_sweep->frequency_shadow = frequency;
	gbz80_apu_init_timer(&frequency_sweep->timer, sweep_time);
	frequency_sweep->enabled = sweep_time != 0 || sweep_shift != 0;

	if (sweep_shift != 0) {
		uint8_t channel_enabled = 0;
		uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift, &channel_enabled);
		return channel_enabled;
	}

	return 1;
}

uint8_t gbz80_apu_frequency_sweep_update(gbz80_apu_frequency_sweep_t* frequency_sweep, uint8_t decrease, uint8_t sweep_shift) {
	uint8_t channel_enabled = 2;
	if (frequency_sweep->enabled && frequency_sweep->timer.period != 0 && gbz80_apu_update_timer(&frequency_sweep->timer)) {
		uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift, &channel_enabled);

		if (channel_enabled && sweep_shift > 0) {
			frequency_sweep->frequency_shadow = new_frequency;
			uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift, &channel_enabled);
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
		0b00000001,
		0b00000011,
		0b00001111,
		0b11111100
	};

	uint8_t wave_output = common_get8_bit(waves[wave_pattern_number], duty_cycler->wave_position);
	
	duty_cycler->wave_position++;
	duty_cycler->wave_position %= 8;

	return wave_output;
}

void gbz80_apu_wave_init(gbz80_apu_wave_cycle_t* wave_cycle)
{
	wave_cycle->wave_position = 0;
}

uint8_t gbz80_apu_wave_update(gbz80_apu_wave_cycle_t* wave_cycle, gbz80_apu_t* apu, uint8_t volume_code)
{
	uint8_t two_samples = gbz80_memory_read8(apu->instance, 0xFF3F - (wave_cycle->wave_position / 2));
	uint8_t bit_start = (wave_cycle->wave_position % 2) * 4;
	uint8_t sample = common_get8_bit_range(two_samples, bit_start, bit_start + 3);
	sample >>= (volume_code + 4) % 5;

	return sample;
}
