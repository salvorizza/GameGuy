#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	#include "common.h"

	#define GBZ80_APU_FREQ 4194304llu

	typedef struct gbz80_t gbz80_t;

	typedef int32_t(*gbz80_apu_sample_function_t)(double, double);
	
		
	typedef struct gbz80_apu_timer_t {
		size_t counter;
		size_t period;
	} gbz80_apu_timer_t;

	typedef struct gbz80_apu_frame_sequencer_t {
		uint8_t num_step;
		uint8_t length_counter_clock;
		uint8_t volume_envelope_clock;
		uint8_t sweep_clock;

		gbz80_apu_timer_t timer;
	} gbz80_apu_frame_sequencer_t;

	typedef struct gbz80_apu_length_counter_t {
		uint8_t counter;
		uint8_t enabled;
	} gbz80_apu_length_counter_t;

	typedef struct gbz80_apu_volume_envelope_t {
		uint8_t counter;
		gbz80_apu_timer_t timer;
	} gbz80_apu_volume_envelope_t;

	typedef struct gbz80_apu_frequency_sweep_t {
		uint8_t enabled;
		uint16_t frequency_shadow;
		gbz80_apu_timer_t timer;
	} gbz80_apu_frequency_sweep_t;

	typedef struct gbz80_apu_duty_cycle_t {
		uint8_t wave_position;
	} gbz80_apu_duty_cycle_t;

	typedef struct gbz80_apu_wave_cycle_t {
		uint8_t wave_position;
	} gbz80_apu_wave_cycle_t;

	typedef struct gbz80_apu_base_channel_t {
		gbz80_apu_length_counter_t length_counter;
		gbz80_apu_volume_envelope_t volume_envelope;
		double dac_output;
		gbz80_apu_timer_t frequency_timer;
	} gbz80_apu_base_channel_t;

	typedef struct gbz80_apu_channel_1_t {
		gbz80_apu_base_channel_t base;
		gbz80_apu_frequency_sweep_t sweeper;
		gbz80_apu_duty_cycle_t duty_cycler;
	} gbz80_apu_channel_1_t;

	typedef struct gbz80_apu_channel_2_t {
		gbz80_apu_base_channel_t base;
		gbz80_apu_duty_cycle_t duty_cycler;
	} gbz80_apu_channel_2_t;

	typedef struct gbz80_apu_channel_3_t {
		gbz80_apu_base_channel_t base;
		gbz80_apu_wave_cycle_t wave_cycler;
	} gbz80_apu_channel_3_t;

	typedef struct gbz80_apu_t {
		double so_1, so_2;
		gbz80_apu_frame_sequencer_t frame_sequencer;
		gbz80_apu_channel_1_t channel_1;
		gbz80_apu_channel_2_t channel_2;
		gbz80_apu_channel_3_t channel_3;

		int32_t status;

		gbz80_apu_timer_t sample_timer;
		gbz80_apu_sample_function_t sample_function;

		gbz80_t* instance;
	} gbz80_apu_t;



	void gbz80_apu_init(gbz80_apu_t* apu, gbz80_t* instance);
	void gbz80_apu_step(gbz80_apu_t* apu, size_t num_cycles);

	uint8_t gbz80_apu_memory_read(gbz80_apu_t* apu, uint16_t address);
	uint8_t gbz80_apu_memory_write(gbz80_apu_t* apu, uint16_t address, uint8_t val);

	void gbz80_apu_trigger_channel1(gbz80_apu_t* apu);
	void gbz80_apu_trigger_channel2(gbz80_apu_t* apu);
	void gbz80_apu_trigger_channel3(gbz80_apu_t* apu);
	void gbz80_apu_trigger_channel4(gbz80_apu_t* apu);

	static __forceinline void gbz80_apu_init_timer(gbz80_apu_timer_t* timer, size_t period) {
		timer->period = period;
		timer->counter = period;
	}

	static __forceinline void gbz80_apu_reset_timer(gbz80_apu_timer_t* timer) {
		timer->counter = timer->period;
	}

	static __forceinline uint8_t gbz80_apu_update_timer(gbz80_apu_timer_t* timer) {
		timer->counter--;
		if (timer->counter == 0) {
			gbz80_apu_reset_timer(timer);
			return 1;
		}
		return 0;
	}

	void gbz80_apu_channel_base_init(gbz80_apu_base_channel_t* channel);

	void gbz80_apu_frame_sequencer_init(gbz80_apu_frame_sequencer_t* frame_sequencer);
	void gbz80_apu_frame_sequencer_update(gbz80_apu_frame_sequencer_t* frame_sequencer);

	void gbz80_apu_length_counter_init(gbz80_apu_length_counter_t* length_counter, uint8_t counter, uint8_t enabled);
	void gbz80_apu_length_counter_update(gbz80_apu_length_counter_t* length_counter,uint8_t length_enable);
	
	void gbz80_apu_volume_envelope_init(gbz80_apu_volume_envelope_t* volume_envelope, uint8_t volume, uint8_t period);
	void gbz80_apu_volume_envelope_update(gbz80_apu_volume_envelope_t* volume_envelope, uint8_t increment);

	uint16_t gbz80_apu_frequency_sweep_calc(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency, uint8_t decrease, uint8_t sweep_shift, uint8_t* channel_enabled);

	uint8_t gbz80_apu_frequency_sweep_init(gbz80_apu_frequency_sweep_t* frequency_sweep, uint16_t frequency, uint8_t sweep_time, uint8_t decrease, uint8_t sweep_shift);
	uint8_t gbz80_apu_frequency_sweep_update(gbz80_apu_frequency_sweep_t* frequency_sweep, uint8_t decrease, uint8_t sweep_shift);

	void gbz80_apu_duty_init(gbz80_apu_duty_cycle_t* duty_cycler);
	uint8_t gbz80_apu_duty_update(gbz80_apu_duty_cycle_t* duty_cycler, uint8_t wave_pattern_number);

	void gbz80_apu_wave_init(gbz80_apu_wave_cycle_t* wave_cycle);
	uint8_t gbz80_apu_wave_update(gbz80_apu_wave_cycle_t* wave_cycle, gbz80_apu_t* apu, uint8_t volume_code);

#ifdef __cplusplus
}
#endif
