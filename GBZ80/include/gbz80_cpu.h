#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

	typedef struct gbz80_t gbz80_t;

	typedef enum gbz80_flag_t {
		GBZ80_FLAG_ZERO = 7,
		GBZ80_FLAG_N = 6,
		GBZ80_FLAG_H = 5,
		GBZ80_FLAG_C = 4
	} gbz80_flag_t;

	typedef enum gbz80_register_t {
		GBZ80_REGISTER_A,
		GBZ80_REGISTER_F,
		GBZ80_REGISTER_B,
		GBZ80_REGISTER_C,
		GBZ80_REGISTER_D,
		GBZ80_REGISTER_E,
		GBZ80_REGISTER_H,
		GBZ80_REGISTER_L,
		GBZ80_REGISTER_AF,
		GBZ80_REGISTER_BC,
		GBZ80_REGISTER_DE,
		GBZ80_REGISTER_HL,
		GBZ80_REGISTER_SP
	} gbz80_register_t;

	typedef enum gbz80_interrupt_type_t {
		GBZ80_INTERRUPT_VBLANK = 0,
		GBZ80_INTERRUPT_LCDSTAT = 1,
		GBZ80_INTERRUPT_TIMER = 2,
		GBZ80_INTERRUPT_SERIAL = 3,
		GBZ80_INTERRUPT_JOYPAD = 4,
		GBZ80_INTERRUPT_MAX
	}gbz80_interrupt_type_t;

	typedef struct gbz80_registers_t {
		union {
			uint16_t AF;
			struct {
				uint8_t flags, A;
			};
		};
		union {
			uint16_t BC;
			struct {
				uint8_t C, B;
			};
		};
		union {
			uint16_t DE;
			struct {
				uint8_t E, D;
			};
		};
		union {
			uint16_t HL;
			struct {
				uint8_t L, H;
			};
		};
		uint16_t SP;
		uint32_t PC;
	} gbz80_registers_t;

	typedef struct gbz80_instruction_t gbz80_instruction_t;

	typedef struct gbz80_cpu_t gbz80_cpu_t;
	
	typedef struct gbz80_instruction_t gbz80_instruction_t;
	typedef void(*gbz80_execute_function_t)(gbz80_cpu_t*, gbz80_instruction_t*);

	typedef struct gbz80_instruction_t {
		uint16_t address;
		uint8_t prefix;
		uint8_t opcode;

		gbz80_register_t left_r;
		gbz80_register_t right_r;
		gbz80_execute_function_t execute_function;

		uint8_t n;
		uint16_t nn;
		int8_t d;

		uint8_t bp;

		uint8_t read_cycle, execution_cycle, write_cycle;
		uint16_t read_address, write_address;

		uint8_t num_total_cycles;
		uint8_t num_current_cycle;

		uint8_t completed;

		char disassembled_name[32];
	} gbz80_instruction_t;

	typedef struct gbz80_cpu_t {
		gbz80_t* instance;
		gbz80_registers_t registers;
		gbz80_instruction_t current_instruction;

		uint8_t dma,dma_enabled;
		uint8_t dma_next_write_address,dma_setup_delay,dma_cycle_count;

		uint8_t ime,ime_ready;
		uint16_t div;
		uint8_t tima, tac, tma;
		uint8_t prev_timer_and_result;
		uint8_t cycles_to_tima_interrupt,cycles_to_tima_interrupt_enable;
		uint8_t read_value;
		uint8_t p1;
		uint8_t sb, sc;
		gbz80_timer_t serial_clock;
		uint8_t serial_div_last,serial_shifts;
		uint8_t interrupt_flag;

		uint8_t halted, halt_bug;

		uint8_t write_db;

		uint8_t wait_cycles;
	} gbz80_cpu_t;

	
	void gbz80_cpu_init(gbz80_cpu_t* cpu, gbz80_t* instance);
	void gbz80_cpu_set_flag(gbz80_cpu_t* cpu, gbz80_flag_t flag, uint8_t val);

	void gbz80_cpu_memory_read(gbz80_cpu_t* cpu, uint16_t address, uint8_t* val);
	uint8_t gbz80_cpu_memory_write(gbz80_cpu_t* cpu, uint16_t address, uint8_t current_value, uint8_t* val);

	void gbz80_cpu_request_interrupt(gbz80_cpu_t* cpu, gbz80_interrupt_type_t interrupt_type);
	uint8_t gbz80_cpu_handle_interrupts(gbz80_cpu_t* cpu);

	uint8_t gbz80_cpu_get_flag(gbz80_cpu_t* cpu, gbz80_flag_t flag);

	void gbz80_cpu_set_register8(gbz80_cpu_t* cpu, gbz80_register_t r, uint8_t val);
	uint8_t gbz80_cpu_get_register8(gbz80_cpu_t* cpu, gbz80_register_t r);
	uint16_t gbz80_cpu_get_register16(gbz80_cpu_t* cpu, gbz80_register_t r);
	void gbz80_cpu_set_register16(gbz80_cpu_t* cpu, gbz80_register_t r, uint16_t val);

	void gbz80_cpu_clock(gbz80_cpu_t* cpu);

	void gbz80_cpu_fetch(gbz80_cpu_t* cpu, gbz80_instruction_t* out_instruction);
	void gbz80_cpu_decode(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction, uint8_t get_instruction_name);
	void gbz80_cpu_execute(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_load8_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load8_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load8_n_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load8_a_hl_dec(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load8_hl_dec_a(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load8_a_hl_inc(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load8_hl_inc_a(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_load16_r_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load16_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load16_nn_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load16_hl_sp_plus_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load16_push(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_load16_pop(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_alu8_add_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_add_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_adc_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_adc_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_sub_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_sub_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_sbc_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_sbc_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_and_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_and_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_or_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_or_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_xor_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_xor_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_cp_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_cp_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_inc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu8_dec_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_alu16_add_hl_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu16_add_sp_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu16_inc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_alu16_dec_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_misc_swap(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_daa(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_cpl(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_ccf(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_scf(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_nop(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_halt(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_stop(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_di(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_misc_ei(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_rtsh_rl_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_rlc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_rla(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_rlca(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_rr_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_rrc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_rra(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_rrca(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_sla_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_sra_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtsh_srl_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_bitw_bit_b_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_bitw_set_b_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_bitw_res_b_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_jumps_jp_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jpnz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jpz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jpnc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jpc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jp_hl(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_jumps_jr_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jrnz_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jrz_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jrnc_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_jumps_jrc_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_calls_call_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_calls_callnz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_calls_callz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_calls_callnc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_calls_callc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_rsts_rst_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);

	void gbz80_cpu_rtrns_ret(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtrns_retnz(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtrns_retz(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtrns_retnc(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtrns_retc(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
	void gbz80_cpu_rtrns_reti(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction);
#ifdef __cplusplus
}
#endif
