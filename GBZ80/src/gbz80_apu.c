#include "gbz80_apu.h"

#include "gbz80.h"

void gbz80_apu_init(gbz80_apu_t* apu, gbz80_t* instance){
	apu->instance = instance;
	gbz80_apu_frame_sequencer_init(&apu->frame_sequencer);
	apu->so_1 = apu->so_2 = 0;
	apu->channel_1.base.dac_output = apu->channel_2.base.dac_output = 0;

}

void gbz80_apu_step(gbz80_apu_t* apu, size_t num_cycles){
	uint8_t nr50 = gbz80_memory_read8(apu->instance, 0xFF20);
	uint8_t nr51 = gbz80_memory_read8(apu->instance, 0xFF25);
	uint8_t nr52 = gbz80_memory_read8(apu->instance, 0xFF26);

	for (size_t i = 0; i < num_cycles; i++) {
		
		gbz80_apu_frame_sequencer_update(&apu->frame_sequencer);

		if (common_get8_bit(nr52, 7) == 1) {
			gbz80_apu_channel_1_clock(apu);
			gbz80_apu_channel_2_clock(apu);
			gbz80_apu_channel_3_clock(apu);
			//gbz80_apu_channel_4_clock(apu);

			double dac_outs[4] = {
				apu->channel_1.base.dac_output,
				apu->channel_2.base.dac_output,
				apu->channel_3.base.dac_output,
				0
			};

			apu->so_1 = apu->so_2 = 0.0;

			for (uint8_t channel_number = 0; channel_number < 4; channel_number++) {
				if (common_get8_bit(nr51, channel_number)) {
					apu->so_1 += dac_outs[channel_number];
				}
			}
			apu->so_1 /= 4.0;

			for (uint8_t channel_number = 0; channel_number < 4; channel_number++) {
				if (common_get8_bit(nr51, 4 + channel_number)) {
					apu->so_2 += dac_outs[channel_number];
				}
			}
			apu->so_2 /= 4.0;

			if (apu->sample_function != NULL) {
				if (apu->sample_timer.period != 0 && gbz80_apu_update_timer(&apu->sample_timer)) {
					apu->sample_function(apu->so_1, apu->so_2);
				}
			}
			
		}
	}
}

void gbz80_apu_channel_1_clock(gbz80_apu_t* apu) {
	if (apu->frame_sequencer.sweep_clock) {
		uint8_t nr10 = gbz80_memory_read8(apu->instance, 0xFF10);

		uint8_t sweep_state = gbz80_apu_frequency_sweep_update(
			&apu->channel_1.sweeper,
			common_get8_bit(nr10, 3),
			common_get8_bit_range(nr10, 0, 2)
		);

		if (sweep_state == 1) {
			uint8_t nr14 = gbz80_memory_read8(apu->instance, 0xFF14);
			common_change8_bit_range(&nr14, 0, 2, (nr14 >> 8) & 0x7);

			gbz80_memory_write8(apu->instance, 0xFF13, (uint8_t)apu->channel_1.sweeper.frequency_shadow);
			gbz80_memory_write8(apu->instance, 0xFF14, nr14);

			apu->channel_1.base.frequency_timer.period = apu->channel_1.sweeper.frequency_shadow;
		}
	}
	else if (apu->frame_sequencer.length_counter_clock) {
		uint8_t nr14 = gbz80_memory_read8(apu->instance, 0xFF14);

		gbz80_apu_length_counter_update(
			&apu->channel_1.base.length_counter,
			common_get8_bit(nr14, 6)
		);
	}
	else if (apu->frame_sequencer.volume_envelope_clock) {
		uint8_t nr12 = gbz80_memory_read8(apu->instance, 0xFF12);

		gbz80_apu_volume_envelope_update(
			&apu->channel_1.base.volume_envelope,
			common_get8_bit(nr12, 3)
		);
	}

	if (gbz80_apu_update_timer(&apu->channel_1.base.frequency_timer)) {
		uint8_t nr11 = gbz80_memory_read8(apu->instance, 0xFF11);
		uint8_t nr12 = gbz80_memory_read8(apu->instance, 0xFF12);

		uint8_t wave_output = gbz80_apu_duty_update(
			&apu->channel_1.duty_cycler,
			common_get8_bit_range(nr11, 6, 7)
		);
		
		wave_output *= apu->channel_1.base.volume_envelope.counter;
		
		if (common_get8_bit_range(nr12, 3, 7) != 0)
			apu->channel_1.base.dac_output = wave_output / 7.5 - 1;
		else
			apu->channel_1.base.dac_output = 0;
	}
}

void gbz80_apu_channel_2_clock(gbz80_apu_t* apu) {
	if (apu->frame_sequencer.length_counter_clock) {
		uint8_t nr24 = gbz80_memory_read8(apu->instance, 0xFF19);

		gbz80_apu_length_counter_update(
			&apu->channel_2.base.length_counter,
			common_get8_bit(nr24, 6)
		);
	}
	else if (apu->frame_sequencer.volume_envelope_clock) {
		uint8_t nr22 = gbz80_memory_read8(apu->instance, 0xFF17);

		gbz80_apu_volume_envelope_update(
			&apu->channel_1.base.volume_envelope,
			common_get8_bit(nr22, 3)
		);
	}

	if (gbz80_apu_update_timer(&apu->channel_2.base.frequency_timer)) {
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
}

void gbz80_apu_channel_3_clock(gbz80_apu_t* apu)
{
	if (apu->frame_sequencer.length_counter_clock) {
		uint8_t nr34 = gbz80_memory_read8(apu->instance, 0xFF1E);

		gbz80_apu_length_counter_update(
			&apu->channel_3.base.length_counter,
			common_get8_bit(nr34, 6)
		);
	}

	if (gbz80_apu_update_timer(&apu->channel_3.base.frequency_timer)) {
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
}

void gbz80_apu_trigger_channel1(gbz80_apu_t* apu) {
	uint8_t nr10 = gbz80_memory_read8(apu->instance, 0xFF10);
	uint8_t nr12 = gbz80_memory_read8(apu->instance, 0xFF12);
	uint8_t nr13 = gbz80_memory_read8(apu->instance, 0xFF13);
	uint8_t nr14 = gbz80_memory_read8(apu->instance, 0xFF14);
	uint16_t frequency = nr13 | ((uint16_t)common_get8_bit_range(nr14, 0, 2) << 8);

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
	gbz80_apu_frequency_sweep_init(
		&apu->channel_1.sweeper,
		frequency,
		common_get8_bit_range(nr10, 4, 6),
		common_get8_bit(nr10, 3),
		common_get8_bit_range(nr10, 0, 2)
	);
	gbz80_apu_duty_init(&apu->channel_1.duty_cycler);
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

void gbz80_apu_init_timer(gbz80_apu_timer_t* timer, size_t period) {
	timer->period = period;
	timer->counter = period;
}

void gbz80_apu_reset_timer(gbz80_apu_timer_t* timer) {
	timer->counter = timer->period;
}

uint8_t gbz80_apu_update_timer(gbz80_apu_timer_t* timer) {
	timer->counter--;
	if (timer->counter == 0) {
		gbz80_apu_reset_timer(timer);
		return 1;
	}
	return 0;
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
		if (increment) {
			uint8_t new_volume = volume_envelope->counter + 1;
			if (new_volume >= 0 && new_volume <= 15) {
				volume_envelope->counter = new_volume;
			}
		}
		else {
			if (volume_envelope->counter != 0) {
				uint8_t new_volume = volume_envelope->counter - 1;
				if (new_volume >= 0 && new_volume <= 15) {
					volume_envelope->counter = new_volume;
				}
			}
		}
	}
}

uint16_t gbz80_apu_frequency_sweep_calc(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency, uint8_t decrease, uint8_t sweep_shift) {
	uint16_t new_frequency = frequency_sweep->frequency_shadow >> sweep_shift;

	if (decrease) {
		new_frequency = frequency_sweep->frequency_shadow - new_frequency;
	}
	else {
		new_frequency = frequency_sweep->frequency_shadow + new_frequency;
	}

	return new_frequency;
}

uint8_t gbz80_apu_frequency_sweep_overflow_check(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency) {
	return frequency > 2047;
}

void gbz80_apu_frequency_sweep_init(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency, uint8_t sweep_time, uint8_t decrease, uint8_t sweep_shift) {
	frequency_sweep->frequency_shadow = frequency;
	gbz80_apu_init_timer(&frequency_sweep->timer, sweep_time);
	frequency_sweep->enabled = sweep_time != 0 || sweep_shift != 0;

	/*if (sweep_shift != 0) {
		uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift);
		if (!gbz80_apu_frequency_sweep_overflow_check(frequency_sweep, new_frequency)) {
			frequency_sweep->frequency_shadow = new_frequency;
			return 1;
		}
		else {
			return 0;
		}
	}*/
}

uint8_t gbz80_apu_frequency_sweep_update(gbz80_apu_frequency_sweep_t* frequency_sweep, uint8_t decrease, uint8_t sweep_shift) {
	if (frequency_sweep->enabled && frequency_sweep->timer.period != 0 && gbz80_apu_update_timer(&frequency_sweep->timer)) {
		uint16_t new_frequency = gbz80_apu_frequency_sweep_calc(frequency_sweep, frequency_sweep->frequency_shadow, decrease, sweep_shift);

		if (!gbz80_apu_frequency_sweep_overflow_check(frequency_sweep, new_frequency)) {
			frequency_sweep->frequency_shadow = new_frequency;
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 2;
	}
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
