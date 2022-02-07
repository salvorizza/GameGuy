#include "gbz80_cpu.h"

#include "gbz80.h"

void gbz80_cpu_init(gbz80_cpu_t* cpu, gbz80_t* instance) {
	memset(cpu, 0, sizeof(gbz80_cpu_t));
	cpu->instance = instance;
	gbz80_init_timer(&cpu->div_timer, GBZ80_CLOCK_HERTZ / 16384);
	gbz80_init_timer(&cpu->tima_timer, 0);
	cpu->ime = 0;
	cpu->ime_ready = 0;
}

void gbz80_cpu_set_flag(gbz80_cpu_t* cpu, gbz80_flag_t flag, uint8_t val) {
	if (val != 0 && val != 1) {
		assert(0 && "Flag has been set to a suspicious value");
		return;
	}
	common_change8_bit(&(cpu->registers.flags), (uint8_t)flag, val);
}

uint8_t gbz80_cpu_memory_read(gbz80_cpu_t* cpu, uint16_t address) {

	return 1;
}


uint8_t gbz80_cpu_memory_write(gbz80_cpu_t* cpu, uint16_t address, uint8_t* val) {
	switch (address) {
		case 0xFF04:
			*val = 0;
			break;

		case 0xFF07:
			if (common_get8_bit(*val, 2) != 0) {
				static size_t cpu_clocks[] = { 4096, 262144, 65536, 16384 };
				uint8_t clock_index = common_get8_bit_range(*val, 0, 1);
				size_t clock_value_hz = cpu_clocks[clock_index];

				gbz80_init_timer(&cpu->tima_timer, GBZ80_CLOCK_HERTZ / clock_value_hz);
			} else {
				gbz80_init_timer(&cpu->tima_timer, 0);
				cpu->tima_timer.period = 0;
			}
			break;

		case 0xFF41: {
			uint8_t prev_lcd_status = gbz80_memory_read8(cpu->instance, 0xFF41);
			uint8_t previous_mode = common_get8_bit_range(prev_lcd_status, 0, 1);
			uint8_t current_mode = common_get8_bit_range(*val, 0, 1);

			if ((previous_mode != current_mode && common_get8_bit(*val, 3 + current_mode) == 1) || (common_get8_bit(*val,6) == 1 && common_get8_bit(*val, 2) == 1)) {
				gbz80_cpu_request_interrupt(cpu, GBZ80_INTERRUPT_LCDSTAT);
			}
			break;
		}
			
	}
	return 1;
}

void gbz80_cpu_request_interrupt(gbz80_cpu_t* cpu, gbz80_interrupt_type_t interrupt_type) {
	uint8_t register_if = gbz80_memory_read_internal(cpu->instance, 0xFF0F);
	common_set8_bit(&register_if, (uint8_t)interrupt_type);
	gbz80_memory_write_internal(cpu->instance, 0xFF0F, register_if);
}

uint8_t gbz80_cpu_handle_interrupts(gbz80_cpu_t* cpu) {
	if (cpu->ime) {
		uint8_t ie = gbz80_memory_read_internal(cpu->instance, 0xFFFF);
		uint8_t register_if = gbz80_memory_read_internal(cpu->instance, 0xFF0F);

		if (ie != 0 && register_if != 0) {
			static uint16_t int_adresses[] = { 0x0040,0x0048,0x0050,0x0058,0x0060 };
			uint8_t not_handled = 1;

			for (uint8_t i = 0; i < (uint8_t)GBZ80_INTERRUPT_MAX; i++) {
				if (common_get8_bit(ie, i) == 1 && common_get8_bit(register_if, i) == 1) {
					common_reset8_bit(&register_if, i);
					gbz80_memory_write_internal(cpu->instance, 0xFF0F, register_if);
					cpu->ime = 0;

					uint16_t int_adress = int_adresses[i];

					uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
					gbz80_memory_write16(cpu->instance, sp - 2, cpu->registers.PC);
					gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp - 2);
					cpu->registers.PC = int_adress;

					cpu->cycles = 20;
					not_handled = 0;
					break;
				}
			}

			return not_handled;
		}
	}

	return 1;
}

uint8_t gbz80_cpu_get_flag(gbz80_cpu_t* cpu, gbz80_flag_t flag){
	return (cpu->registers.flags >> (uint8_t)flag) & 0x1;
}

void gbz80_cpu_set_register8(gbz80_cpu_t* cpu, gbz80_register_t r, uint8_t val) {
	switch (r)
	{
		case GBZ80_REGISTER_A :
			cpu->registers.A = val;
			break;
		case GBZ80_REGISTER_F:
			cpu->registers.flags = val;
			break;
		case GBZ80_REGISTER_B:
			cpu->registers.B = val;
			break;
		case GBZ80_REGISTER_C:
			cpu->registers.C = val;
			break;
		case GBZ80_REGISTER_D:
			cpu->registers.D = val;
			break;
		case GBZ80_REGISTER_E:
			cpu->registers.E = val;
			break;
		case GBZ80_REGISTER_H:
			cpu->registers.H= val;
			break;
		case GBZ80_REGISTER_L:
			cpu->registers.L = val;
			break;

		case GBZ80_REGISTER_AF:
			gbz80_memory_write8(cpu->instance, cpu->registers.AF, val);
			break;

		case GBZ80_REGISTER_BC:
			gbz80_memory_write8(cpu->instance, cpu->registers.BC, val);
			break;

		case GBZ80_REGISTER_DE:
			gbz80_memory_write8(cpu->instance, cpu->registers.DE, val);
			break;

		case GBZ80_REGISTER_HL:
			gbz80_memory_write8(cpu->instance, cpu->registers.HL, val);
			break;
	default:
		break;
	}
}

uint8_t gbz80_cpu_get_register8(gbz80_cpu_t* cpu, gbz80_register_t r) {
	switch (r)
	{
	case GBZ80_REGISTER_A:
		return cpu->registers.A;

	case GBZ80_REGISTER_F:
		return cpu->registers.flags;

	case GBZ80_REGISTER_B:
		return cpu->registers.B;

	case GBZ80_REGISTER_C:
		return cpu->registers.C;

	case GBZ80_REGISTER_D:
		return cpu->registers.D;

	case GBZ80_REGISTER_E:
		return cpu->registers.E;

	case GBZ80_REGISTER_H:
		return cpu->registers.H;

	case GBZ80_REGISTER_L:
		return cpu->registers.L;

	case GBZ80_REGISTER_AF:
		return gbz80_memory_read8(cpu->instance, cpu->registers.AF);

	case GBZ80_REGISTER_BC:
		return gbz80_memory_read8(cpu->instance, cpu->registers.BC);

	case GBZ80_REGISTER_DE:
		return gbz80_memory_read8(cpu->instance, cpu->registers.DE);

	case GBZ80_REGISTER_HL:
		return gbz80_memory_read8(cpu->instance, cpu->registers.HL);
	}

	return 0;
}

uint16_t gbz80_cpu_get_register16(gbz80_cpu_t* cpu, gbz80_register_t r) {
	switch (r)
	{
		case GBZ80_REGISTER_AF:
			return cpu->registers.AF;

		case GBZ80_REGISTER_BC:
			return cpu->registers.BC;

		case GBZ80_REGISTER_DE:
			return cpu->registers.DE;

		case GBZ80_REGISTER_HL:
			return cpu->registers.HL;

		case GBZ80_REGISTER_SP:
			return cpu->registers.SP;
	}

	return 0;
}

void gbz80_cpu_set_register16(gbz80_cpu_t* cpu, gbz80_register_t r, uint16_t val) {
	switch (r)
	{
	case GBZ80_REGISTER_AF:
		cpu->registers.AF = val;
		break;

	case GBZ80_REGISTER_BC:
		cpu->registers.BC = val;
		break;

	case GBZ80_REGISTER_DE:
		cpu->registers.DE = val;
		break;

	case GBZ80_REGISTER_HL:
		cpu->registers.HL = val;
		break;

	case GBZ80_REGISTER_SP:
		cpu->registers.SP = val;
		break;
	}
}

void gbz80_cpu_clock(gbz80_cpu_t* cpu)
{
	if (cpu->div_timer.period != 0 && gbz80_update_timer(&cpu->div_timer)) {
		uint8_t div = gbz80_memory_read_internal(cpu->instance, 0xFF04);
		gbz80_memory_write8(cpu->instance, 0xFF04, div + 1);
	}

	if (cpu->tima_timer.period != 0 && gbz80_update_timer(&cpu->tima_timer)) {
		uint16_t tima = gbz80_memory_read_internal(cpu->instance, 0xFF05);
		tima++;
		if (tima <= 0xFF) {
			gbz80_memory_write_internal(cpu->instance, 0xFF05, (uint8_t)tima);
		}
		else {
			uint8_t tma = gbz80_memory_read_internal(cpu->instance, 0xFF06);
			gbz80_memory_write_internal(cpu->instance, 0xFF05, tma);
			gbz80_cpu_request_interrupt(cpu, GBZ80_INTERRUPT_TIMER);
		}
	}

	if (cpu->cycles == 0) {
		if (gbz80_cpu_handle_interrupts(cpu) == 1) {
			uint8_t ime_ready_prev = cpu->ime_ready;

			memset(&cpu->current_instruction, 0, sizeof(gbz80_instruction_t));
			gbz80_cpu_fetch(cpu, &cpu->current_instruction);
			gbz80_cpu_decode(cpu, &cpu->current_instruction, 0);

			cpu->cycles = gbz80_cpu_execute(cpu, &cpu->current_instruction);

			if (ime_ready_prev == 1 && cpu->ime_ready == 1) {
				cpu->ime = 1;
			}
		}
	}
	cpu->cycles--;
}

void gbz80_cpu_fetch(gbz80_cpu_t* cpu, gbz80_instruction_t* out_instruction)
{
	out_instruction->address = cpu->registers.PC;
	uint8_t opcode = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
	out_instruction->prefix = 0x00;
	if (opcode == 0xCB) {
		out_instruction->prefix = opcode;
		opcode = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
	}
	out_instruction->opcode = opcode;
}

void gbz80_cpu_decode(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction, uint8_t get_instruction_name) {
	uint8_t opcode = instruction->opcode;
	uint8_t prefix = instruction->prefix;

	if (prefix == 0x00 && (opcode == 0x06 || opcode == 0x0E || opcode == 0x16 || opcode == 0x1E || opcode == 0x26 || opcode == 0x2E)) {
		instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
		instruction->execute_function = &gbz80_cpu_load8_r_n;
		instruction->cycles = 8;

		switch (instruction->opcode)
		{
			case 0x06:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,$%02X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_B;
				break;

			case 0x0E:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,$%02X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_C;
				break;

			case 0x16:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,$%02X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_D;
				break;

			case 0x1E:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,$%02X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_E;
				break;

			case 0x26:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H$%02X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_H;
				break;

			case 0x2E:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,$%02X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_L;
				break;
		}
	}
	else if (	prefix == 0x00 && (opcode == 0x7F || opcode == 0x78 || opcode == 0x79 || opcode == 0x7A || opcode == 0x7B || opcode == 0x7C || opcode == 0x7D || opcode == 0x7E || opcode == 0x40 || opcode == 0x41 || 
				opcode == 0x42 || opcode == 0x43 || opcode == 0x44 || opcode == 0x45 || opcode == 0x46 || opcode == 0x47 || opcode == 0x48 || opcode == 0x49 || opcode == 0x4A || opcode == 0x4B || opcode == 0x4C || 
				opcode == 0x4D || opcode == 0x4E || opcode == 0x4F || opcode == 0x50 || opcode == 0x51 || opcode == 0x52 || opcode == 0x53 || opcode == 0x54 || opcode == 0x55 || opcode == 0x56 || opcode == 0x57 ||
				opcode == 0x58 || opcode == 0x59 || opcode == 0x5A || opcode == 0x5B || opcode == 0x5C || opcode == 0x5D || opcode == 0x5E || opcode == 0x5F || opcode == 0x60 || opcode == 0x61 || opcode == 0x62 || 
				opcode == 0x63 || opcode == 0x64 || opcode == 0x65 || opcode == 0x66 || opcode == 0x67 || opcode == 0x68 || opcode == 0x69 || opcode == 0x6A || opcode == 0x6B || opcode == 0x6C || opcode == 0x6D || 
				opcode == 0x6E || opcode == 0x6F || opcode == 0x70 || opcode == 0x71 || opcode == 0x72 || opcode == 0x73 || opcode == 0x74 || opcode == 0x75 || opcode == 0x36)) {
			instruction->execute_function = &gbz80_cpu_load8_r_r;
			instruction->cycles = 4;

			switch (opcode) {
				case 0x7F:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,A");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x78:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,B");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x79:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,C");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x7A:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,D");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x7B:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,E");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x7C:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,H");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x7D:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,L");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x7E:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,(HL)");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x40:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,B");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x41:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,C");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x42:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,D");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x43:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,E");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x44:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,H");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x45:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,L");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x46:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,(HL)");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x47:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD B,A");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x48:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,B");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x49:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,C");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x4A:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,D");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x4B:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,E");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x4C:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,H");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x4D:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,L");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x4E:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,(HL)");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x4F:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD C,A");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x50:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,B");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x51:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,C");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x52:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,D");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x53:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,E");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x54:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,H");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x55:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,L");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x56:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,(HL)");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x57:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD D,A");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x58:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,B");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x59:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,C");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x5A:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,D");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x5B:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,E");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x5C:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,H");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x5D:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,L");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x5E:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,(HL)");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x5F:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD E,A");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x60:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,B");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x61:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,C");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x62:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,D");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x63:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,E");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x64:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,H");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x65:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,L");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x66:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,(HL)");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x67:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD H,A");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x68:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,B");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x69:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,C");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x6A:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,D");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x6B:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,E");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x6C:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,H");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x6D:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,L");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x6E:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,(HL)");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x6F:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD L,A");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x70:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),B");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_B;
					instruction->cycles = 8;
					break;

				case 0x71:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),C");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_C;
					instruction->cycles = 8;
					break;

				case 0x72:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),D");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_D;
					instruction->cycles = 8;
					break;

				case 0x73:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),E");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_E;
					instruction->cycles = 8;
					break;

				case 0x74:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),H");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_H;
					instruction->cycles = 8;
					break;

				case 0x75:
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),L");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_L;
					instruction->cycles = 8;
					break;

				case 0x36:
					instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
					if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),$%02X", instruction->n);
					instruction->execute_function = &gbz80_cpu_load8_r_n;
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->cycles = 12;
					break;

			}
	} 
	else if (prefix == 0x00 && (opcode == 0x0A || opcode == 0x1A || opcode == 0x7E || opcode == 0xFA || opcode == 0x3E)) {
		instruction->execute_function = &gbz80_cpu_load8_r_r;
		instruction->cycles = 8;
		switch (opcode)
		{
			case 0x0A:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,(BC)");
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_BC;
				break;

			case 0x1A:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,(DE)");
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_DE;
				break;

			case 0x7E:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,(HL)");
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				break;

			case 0xFA: {
				uint16_t nn = gbz80_memory_read16(cpu->instance, cpu->registers.PC);
				instruction->n = gbz80_memory_read8(cpu->instance, nn);
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,($%04X)", nn);
				instruction->left_r= GBZ80_REGISTER_A;
				instruction->execute_function = &gbz80_cpu_load8_r_n;
				instruction->cycles = 16;
				cpu->registers.PC += 2;
				break;
			}

			case 0x3E:
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,$%02X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->execute_function = &gbz80_cpu_load8_r_n;
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x02 || opcode == 0x12 || opcode == 0x77 || opcode == 0xEA)) {
		instruction->execute_function = &gbz80_cpu_load8_r_r;
		instruction->cycles = 8;
		switch (opcode)
		{
			case 0x02:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (BC),A");
				instruction->left_r = GBZ80_REGISTER_BC;
				instruction->right_r = GBZ80_REGISTER_A;
				break;

			case 0x12:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (DE),A");
				instruction->left_r = GBZ80_REGISTER_DE;
				instruction->right_r = GBZ80_REGISTER_A;
				break;

			case 0x77:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL),A");
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->right_r = GBZ80_REGISTER_A;
				break;

			case 0xEA: {
				instruction->nn = gbz80_memory_read16(cpu->instance, cpu->registers.PC);
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD ($%04X),A", instruction->nn);
				instruction->right_r = GBZ80_REGISTER_A;
				instruction->execute_function = &gbz80_cpu_load8_n_r;
				instruction->cycles = 16;
				cpu->registers.PC += 2;
				break;
			}
		}
	}
	else if (prefix == 0x00 && (opcode == 0xF2)) {
		instruction->execute_function = &gbz80_cpu_load8_r_n;
		instruction->cycles = 8;
		instruction->n = gbz80_memory_read8(cpu->instance, 0xFF00 + cpu->registers.C);
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,($FF00+C)");
		instruction->left_r = GBZ80_REGISTER_A;
	}
	else if (prefix == 0x00 && (opcode == 0xE2)) {
		instruction->execute_function = &gbz80_cpu_load8_n_r;
		instruction->cycles = 8;
		instruction->nn = 0xFF00 + cpu->registers.C;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD ($FF00+C),A");
		instruction->right_r = GBZ80_REGISTER_A;
	}
	else if (prefix == 0x00 && (opcode == 0x3A)) {
		instruction->execute_function = &gbz80_cpu_load8_a_hl_dec;
		instruction->cycles = 8;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,(HL-)");
	}
	else if (prefix == 0x00 && (opcode == 0x32)) {
		instruction->execute_function = &gbz80_cpu_load8_hl_dec_a;
		instruction->cycles = 8;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL-),A");
	}
	else if (prefix == 0x00 && (opcode == 0x2A)) {
		instruction->execute_function = &gbz80_cpu_load8_a_hl_inc;
		instruction->cycles = 8;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,(HL+)");
	}
	else if (prefix == 0x00 && (opcode == 0x22)) {
		instruction->execute_function = &gbz80_cpu_load8_hl_inc_a;
		instruction->cycles = 8;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD (HL+),A");
	}
	else if (prefix == 0x00 && (opcode == 0xE0)) {
		instruction->execute_function = &gbz80_cpu_load8_n_r;
		instruction->cycles = 12;
		uint8_t n = gbz80_memory_read8(cpu->instance,cpu->registers.PC++);
		instruction->nn = 0xFF00 + n;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD ($FF00+$%02X),A",n);
		instruction->right_r = GBZ80_REGISTER_A;
	}
	else if (prefix == 0x00 && (opcode == 0xF0)) {
		instruction->execute_function = &gbz80_cpu_load8_r_n;
		instruction->cycles = 12;
		uint8_t n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
		instruction->n = gbz80_memory_read8(cpu->instance, 0xFF00 + n);
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD A,($FF00+$%02X)", n);
		instruction->right_r = GBZ80_REGISTER_A;
	}
	else if (prefix == 0x00 && (opcode == 0x01 || opcode == 0x11 || opcode == 0x21 || opcode == 0x31)) {
		instruction->execute_function = &gbz80_cpu_load16_r_nn;
		instruction->cycles = 12;
		instruction->nn = gbz80_memory_read16(cpu->instance, cpu->registers.PC);
		cpu->registers.PC += 2;

		switch (opcode) {
			case 0x01:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD BC,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_BC;
				break;

			case 0x11:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD DE,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_DE;
				break;

			case 0x21:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD HL,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_HL;
				break;

			case 0x31:
				if(get_instruction_name) sprintf(instruction->disassembled_name, "LD SP,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_SP;
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xF9)) {
		instruction->execute_function = &gbz80_cpu_load16_r_r;
		instruction->cycles = 8;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD SP,HL");
		instruction->left_r = GBZ80_REGISTER_SP;
		instruction->right_r = GBZ80_REGISTER_HL;
	}
	else if (prefix == 0x00 && (opcode == 0xF8)) {
		instruction->execute_function = &gbz80_cpu_load16_hl_sp_plus_n;
		instruction->cycles = 12;
		instruction->d = (int8_t)gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LDHL SP,$%04X", instruction->d);
	}
	else if (prefix == 0x00 && (opcode == 0x08)) {
		instruction->execute_function = &gbz80_cpu_load16_nn_r;
		instruction->cycles = 20;
		instruction->nn = gbz80_memory_read16(cpu->instance, cpu->registers.PC);
		cpu->registers.PC += 2;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "LD ($%04X),SP", instruction->nn);
		instruction->right_r = GBZ80_REGISTER_SP;
	}
	else if (prefix == 0x00 && (opcode == 0xF5 || opcode == 0xC5 || opcode == 0xD5 || opcode == 0xE5)) {
		instruction->execute_function = &gbz80_cpu_load16_push;
		instruction->cycles = 16;

		switch (opcode) {
			case 0xF5:
				instruction->right_r = GBZ80_REGISTER_AF;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "PUSH AF");
				break;

			case 0xC5:
				instruction->right_r = GBZ80_REGISTER_BC;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "PUSH BC");
				break;

			case 0xD5:
				instruction->right_r = GBZ80_REGISTER_DE;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "PUSH DE");
				break;

			case 0xE5:
				instruction->right_r = GBZ80_REGISTER_HL;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "PUSH HL");
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xF1 || opcode == 0xC1 || opcode == 0xD1 || opcode == 0xE1)) {
		instruction->execute_function = &gbz80_cpu_load16_pop;
		instruction->cycles = 12;

		switch (opcode) {
		case 0xF1:
			instruction->left_r = GBZ80_REGISTER_AF;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "POP AF");
			break;

		case 0xC1:
			instruction->left_r = GBZ80_REGISTER_BC;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "POP BC");
			break;

		case 0xD1:
			instruction->left_r = GBZ80_REGISTER_DE;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "POP DE");
			break;

		case 0xE1:
			instruction->left_r = GBZ80_REGISTER_HL;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "POP HL");
			break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x87 || opcode == 0x80 || opcode == 0x81 || opcode == 0x82 || opcode == 0x83 || opcode == 0x84 || opcode == 0x85 || opcode == 0x86 || opcode == 0xC6)) {
		instruction->execute_function = &gbz80_cpu_alu8_add_r_r;
		instruction->cycles = 4;
		
		switch (opcode) {
			case 0x87:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,A");
				break;

			case 0x80:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,B");
				break;

			case 0x81:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,C");
				break;

			case 0x82:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,D");
				break;

			case 0x83:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,E");
				break;

			case 0x84:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,H");
				break;

			case 0x85:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,L");
				break;

			case 0x86:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,(HL)");
				break;

			case 0xC6:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_add_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD A,$%02X", instruction->n);
				break;
			
		}
	}
	else if (prefix == 0x00 && (opcode == 0x8F || opcode == 0x88 || opcode == 0x89 || opcode == 0x8A || opcode == 0x8B || opcode == 0x8C || opcode == 0x8D || opcode == 0x8E || opcode == 0xCE)) {
		instruction->execute_function = &gbz80_cpu_alu8_adc_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0x8F:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,A");
				break;

			case 0x88:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,B");
				break;

			case 0x89:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,C");
				break;

			case 0x8A:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,D");
				break;

			case 0x8B:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,E");
				break;

			case 0x8C:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,H");
				break;

			case 0x8D:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,L");
				break;

			case 0x8E:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,(HL)");
				break;

			case 0xCE:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_adc_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADC A,$%02X", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x97 || opcode == 0x90 || opcode == 0x91 || opcode == 0x92 || opcode == 0x93 || opcode == 0x94 || opcode == 0x95 || opcode == 0x96 || opcode == 0xD6)) {
		instruction->execute_function = &gbz80_cpu_alu8_sub_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0x97:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB A");
				break;

			case 0x90:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB B");
				break;

			case 0x91:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB C");
				break;

			case 0x92:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB D");
				break;

			case 0x93:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB E");
				break;

			case 0x94:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB H");
				break;

			case 0x95:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB L");
				break;

			case 0x96:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB (HL)");
				break;

			case 0xD6:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_sub_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SUB $%02X", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x9F || opcode == 0x98 || opcode == 0x99 || opcode == 0x9A || opcode == 0x9B || opcode == 0x9C || opcode == 0x9D || opcode == 0x9E || opcode == 0xDE)) {
		instruction->execute_function = &gbz80_cpu_alu8_sbc_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0x9F:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,A");
				break;

			case 0x98:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,B");
				break;

			case 0x99:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,C");
				break;

			case 0x9A:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,D");
				break;

			case 0x9B:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,E");
				break;

			case 0x9C:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,H");
				break;

			case 0x9D:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,L");
				break;

			case 0x9E:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,(HL)");
				break;

			case 0xDE:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_sbc_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SBC A,$%02X", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xA7 || opcode == 0xA0 || opcode == 0xA1 || opcode == 0xA2 || opcode == 0xA3 || opcode == 0xA4 || opcode == 0xA5 || opcode == 0xA6 || opcode == 0xE6)) {
		instruction->execute_function = &gbz80_cpu_alu8_and_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0xA7:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND A");
				break;

			case 0xA0:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND B");
				break;

			case 0xA1:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND C");
				break;

			case 0xA2:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND D");
				break;

			case 0xA3:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND E");
				break;

			case 0xA4:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND H");
				break;

			case 0xA5:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND L");
				break;

			case 0xA6:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND (HL)");
				break;

			case 0xE6:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_and_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "AND $%02X", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xB7 || opcode == 0xB0 || opcode == 0xB1 || opcode == 0xB2 || opcode == 0xB3 || opcode == 0xB4 || opcode == 0xB5 || opcode == 0xB6 || opcode == 0xF6)) {
		instruction->execute_function = &gbz80_cpu_alu8_or_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0xB7:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR A");
				break;

			case 0xB0:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR B");
				break;

			case 0xB1:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR C");
				break;

			case 0xB2:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR D");
				break;

			case 0xB3:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR E");
				break;

			case 0xB4:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR H");
				break;

			case 0xB5:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR L");
				break;

			case 0xB6:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR (HL)");
				break;

			case 0xF6:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_or_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "OR $%02X", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xAF || opcode == 0xA8 || opcode == 0xA9 || opcode == 0xAA || opcode == 0xAB || opcode == 0xAC || opcode == 0xAD || opcode == 0xAE || opcode == 0xEE)) {
		instruction->execute_function = &gbz80_cpu_alu8_xor_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0xAF:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR A");
				break;

			case 0xA8:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR B");
				break;

			case 0xA9:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR C");
				break;

			case 0xAA:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR D");
				break;

			case 0xAB:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR E");
				break;

			case 0xAC:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR H");
				break;

			case 0xAD:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR L");
				break;

			case 0xAE:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR (HL)");
				break;

			case 0xEE:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_xor_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR $%02X", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xBF || opcode == 0xB8 || opcode == 0xB9 || opcode == 0xBA || opcode == 0xBB || opcode == 0xBC || opcode == 0xBD || opcode == 0xBE || opcode == 0xFE)) {
		instruction->execute_function = &gbz80_cpu_alu8_cp_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0xBF:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP A");
				break;

			case 0xB8:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP B");
				break;

			case 0xB9:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "XOR C");
				break;

			case 0xBA:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP D");
				break;

			case 0xBB:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP E");
				break;

			case 0xBC:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP H");
				break;

			case 0xBD:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP L");
				break;

			case 0xBE:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP (HL)");
				break;

			case 0xFE:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_cp_r_n;
				instruction->cycles = 8;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CP $%02X", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x3C || opcode == 0x04 || opcode == 0x0C || opcode == 0x14 || opcode == 0x1C || opcode == 0x24 || opcode == 0x2C || opcode == 0x34)) {
		instruction->execute_function = &gbz80_cpu_alu8_inc_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0x3C:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC A");
				break;

			case 0x04:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC B");
				break;

			case 0x0C:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC C");
				break;

			case 0x14:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC D");
				break;

			case 0x1C:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC E");
				break;

			case 0x24:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC H");
				break;

			case 0x2C:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC L");
				break;

			case 0x34:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 12;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC (HL)");
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x3D || opcode == 0x05 || opcode == 0x0D || opcode == 0x15 || opcode == 0x1D || opcode == 0x25 || opcode == 0x2D || opcode == 0x35)) {
		instruction->execute_function = &gbz80_cpu_alu8_dec_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0x3D:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC A");
				break;

			case 0x05:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC B");
				break;

			case 0x0D:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC C");
				break;

			case 0x15:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC D");
				break;

			case 0x1D:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC E");
				break;

			case 0x25:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC H");
				break;

			case 0x2D:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC L");
				break;

			case 0x35:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 12;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC (HL)");
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x09 || opcode == 0x19 || opcode == 0x29 || opcode == 0x39)) {
		instruction->execute_function = &gbz80_cpu_alu16_add_hl_r;
		instruction->cycles = 8;
		switch (opcode) {
			case 0x09:
				instruction->right_r = GBZ80_REGISTER_BC;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD HL,BC");
				break;

			case 0x19:
				instruction->right_r = GBZ80_REGISTER_DE;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD HL,DE");
				break;

			case 0x29:
				instruction->right_r = GBZ80_REGISTER_HL;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD HL,HL");
				break;

			case 0x39:
				instruction->right_r = GBZ80_REGISTER_SP;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD HL,SP");
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xE8)) {
		instruction->execute_function = &gbz80_cpu_alu16_add_sp_nn;
		instruction->cycles = 16;
		instruction->d = (int8_t)gbz80_memory_read8(cpu->instance,cpu->registers.PC++);
		if(get_instruction_name) sprintf(instruction->disassembled_name, "ADD SP,$%04X", instruction->d);
	}
	else if (prefix == 0x00 && (opcode == 0x03 || opcode == 0x13 || opcode == 0x23 || opcode == 0x33)) {
		instruction->execute_function = &gbz80_cpu_alu16_inc_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x03:
				instruction->left_r = GBZ80_REGISTER_BC;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC BC");
				break;

			case 0x13:
				instruction->left_r = GBZ80_REGISTER_DE;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC DE");
				break;

			case 0x23:
				instruction->left_r = GBZ80_REGISTER_HL;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC HL");
				break;

			case 0x33:
				instruction->left_r = GBZ80_REGISTER_SP;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "INC SP");
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x0B || opcode == 0x1B || opcode == 0x2B || opcode == 0x3B)) {
		instruction->execute_function = &gbz80_cpu_alu16_dec_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x0B:
				instruction->left_r = GBZ80_REGISTER_BC;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC BC");
				break;

			case 0x1B:
				instruction->left_r = GBZ80_REGISTER_DE;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC DE");
				break;

			case 0x2B:
				instruction->left_r = GBZ80_REGISTER_HL;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC HL");
				break;

			case 0x3B:
				instruction->left_r = GBZ80_REGISTER_SP;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "DEC SP");
				break;
		}
	}
	else if (prefix == 0xCB && (opcode == 0x37 || opcode == 0x30 || opcode == 0x31 || opcode == 0x32 || opcode == 0x33 || opcode == 0x34 || opcode == 0x35 || opcode == 0x36)) {
		instruction->execute_function = &gbz80_cpu_misc_swap;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x37:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP A");
				break;

			case 0x30:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP B");
				break;

			case 0x31:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP C");
				break;

			case 0x32:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP D");
				break;

			case 0x33:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP E");
				break;

			case 0x34:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP H");
				break;

			case 0x35:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP L");
				break;

			case 0x36:
				instruction->left_r = GBZ80_REGISTER_HL;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SWAP (HL)");
				instruction->cycles = 16;
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0x27)) {
		instruction->execute_function = &gbz80_cpu_misc_daa;
		instruction->cycles = 4;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "DAA");
	}
	else if (prefix == 0x00 && (opcode == 0x2F)) {
		instruction->execute_function = &gbz80_cpu_misc_cpl;
		instruction->cycles = 4;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "CPL");
	}
	else if (prefix == 0x00 && (opcode == 0x3F)) {
		instruction->execute_function = &gbz80_cpu_misc_ccf;
		instruction->cycles = 4;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "CCF");
	}
	else if (prefix == 0x00 && (opcode == 0x37)) {
		instruction->execute_function = &gbz80_cpu_misc_scf;
		instruction->cycles = 4;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "SCF");
	}
	else if (prefix == 0x00 && (opcode == 0x00)) {
		instruction->execute_function = &gbz80_cpu_misc_nop;
		instruction->cycles = 4;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "NOP");
	}
	else if (prefix == 0x00 && (opcode == 0x10)) {
		uint8_t n = gbz80_memory_read8(cpu->instance, cpu->registers.PC++);
		if (n == 0x00) {
			instruction->execute_function = &gbz80_cpu_misc_stop;
			instruction->cycles = 4;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "STOP");
		}
	}
	else if (prefix == 0x00 && (opcode == 0xF3)) {
		instruction->execute_function = &gbz80_cpu_misc_di;
		instruction->cycles = 4;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "DI");
	}
	else if (prefix == 0x00 && (opcode == 0xFB)) {
		instruction->execute_function = &gbz80_cpu_misc_ei;
		instruction->cycles = 4;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "EI");
	}
	else if (prefix == 0x00 && (opcode == 0x07)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rlca;
		instruction->cycles = 4;
		instruction->left_r = GBZ80_REGISTER_A;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "RLCA");
	}
	else if (prefix == 0x00 && (opcode == 0x17)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rla;
		instruction->cycles = 4;
		instruction->left_r = GBZ80_REGISTER_A;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "RLA");
	}
	else if (prefix == 0x00 && (opcode == 0x0F)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rrca;
		instruction->cycles = 4;
		instruction->left_r = GBZ80_REGISTER_A;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "RRCA");
	}
	else if (prefix == 0x00 && (opcode == 0x1F)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rra;
		instruction->cycles = 4;
		instruction->left_r = GBZ80_REGISTER_A;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "RRA");
	}
	else if (prefix == 0xCB && (opcode == 0x07 || opcode == 0x00 || opcode == 0x01 || opcode == 0x02 || opcode == 0x03 || opcode == 0x04 || opcode == 0x05 || opcode == 0x06)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rlc_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x07:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC A");
				break;

			case 0x00:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC B");
				break;

			case 0x01:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC C");
				break;

			case 0x02:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC D");
				break;

			case 0x03:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC E");
				break;

			case 0x04:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC H");
				break;

			case 0x05:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC L");
				break;

			case 0x06:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RLC (HL)");
				break;
		}
	}
	else if (prefix == 0xCB && (opcode == 0x17 || opcode == 0x10 || opcode == 0x11 || opcode == 0x12 || opcode == 0x13 || opcode == 0x14 || opcode == 0x15 || opcode == 0x16)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rl_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x17:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL A");
				break;

			case 0x10:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL B");
				break;

			case 0x11:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL C");
				break;

			case 0x12:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL D");
				break;

			case 0x13:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL E");
				break;

			case 0x14:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL H");
				break;

			case 0x15:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL L");
				break;

			case 0x16:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RL (HL)");
				break;
		}
	}
	else if (prefix == 0xCB && (opcode == 0x0F || opcode == 0x08 || opcode == 0x09 || opcode == 0x0A || opcode == 0x0B || opcode == 0x0C || opcode == 0x0D || opcode == 0x0E)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rrc_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x0F:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC A");
				break;

			case 0x08:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC B");
				break;

			case 0x09:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC C");
				break;

			case 0x0A:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC D");
				break;

			case 0x0B:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC E");
				break;

			case 0x0C:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC H");
				break;

			case 0x0D:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC L");
				break;

			case 0x0E:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RRC (HL)");
				break;
		}
	}
	else if (prefix == 0xCB && (opcode == 0x1F || opcode == 0x18 || opcode == 0x19 || opcode == 0x1A || opcode == 0x1B || opcode == 0x1C || opcode == 0x1D || opcode == 0x1E)) {
		instruction->execute_function = &gbz80_cpu_rtsh_rr_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x1F:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR A");
				break;

			case 0x18:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR B");
				break;

			case 0x19:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR C");
				break;

			case 0x1A:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR D");
				break;

			case 0x1B:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR E");
				break;

			case 0x1C:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR H");
				break;

			case 0x1D:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR L");
				break;

			case 0x1E:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RR (HL)");
				break;
		}
	}
	else if (prefix == 0xCB && (opcode == 0x27 || opcode == 0x20 || opcode == 0x21 || opcode == 0x22 || opcode == 0x23 || opcode == 0x24 || opcode == 0x25 || opcode == 0x26)) {
		instruction->execute_function = &gbz80_cpu_rtsh_sla_r;
		instruction->cycles = 8;

		switch (opcode) {
		case 0x27:
			instruction->left_r = GBZ80_REGISTER_A;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA A");
			break;

		case 0x20:
			instruction->left_r = GBZ80_REGISTER_B;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA B");
			break;

		case 0x21:
			instruction->left_r = GBZ80_REGISTER_C;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA C");
			break;

		case 0x22:
			instruction->left_r = GBZ80_REGISTER_D;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA D");
			break;

		case 0x23:
			instruction->left_r = GBZ80_REGISTER_E;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA E");
			break;

		case 0x24:
			instruction->left_r = GBZ80_REGISTER_H;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA H");
			break;

		case 0x25:
			instruction->left_r = GBZ80_REGISTER_L;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA L");
			break;

		case 0x26:
			instruction->left_r = GBZ80_REGISTER_HL;
			instruction->cycles = 16;
			if(get_instruction_name) sprintf(instruction->disassembled_name, "SLA (HL)");
			break;
		}
	}
	else if (prefix == 0xCB && (opcode == 0x2F || opcode == 0x28 || opcode == 0x29 || opcode == 0x2A || opcode == 0x2B || opcode == 0x2C || opcode == 0x2D || opcode == 0x2E)) {
		instruction->execute_function = &gbz80_cpu_rtsh_sra_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x2F:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA A");
				break;

			case 0x28:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA B");
				break;

			case 0x29:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA C");
				break;

			case 0x2A:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA D");
				break;

			case 0x2B:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA E");
				break;

			case 0x2C:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA H");
				break;

			case 0x2D:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA L");
				break;

			case 0x2E:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRA (HL)");
				break;
		}
	}
	else if (prefix == 0xCB && (opcode == 0x3F || opcode == 0x38 || opcode == 0x39 || opcode == 0x3A || opcode == 0x3B || opcode == 0x3C || opcode == 0x3D || opcode == 0x3E)) {
		instruction->execute_function = &gbz80_cpu_rtsh_srl_r;
		instruction->cycles = 8;

		switch (opcode) {
			case 0x3F:
				instruction->left_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL A");
				break;

			case 0x38:
				instruction->left_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL B");
				break;

			case 0x39:
				instruction->left_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL C");
				break;

			case 0x3A:
				instruction->left_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL D");
				break;

			case 0x3B:
				instruction->left_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL E");
				break;

			case 0x3C:
				instruction->left_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL H");
				break;

			case 0x3D:
				instruction->left_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL L");
				break;

			case 0x3E:
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SRL (HL)");
				break;
		}
	}
	else if (prefix == 0xCB && (opcode >= 0x40 && opcode <= 0x7F)) {
		instruction->execute_function = &gbz80_cpu_bitw_bit_b_r;
		instruction->cycles = 8;
		instruction->n = (opcode >> 3) & 0x7;

		switch (opcode & 0xC7) {
			case 0x47:
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,A", instruction->n);
				break;

			case 0x40:
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,B", instruction->n);
				break;

			case 0x41:
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,C", instruction->n);
				break;

			case 0x42:
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,D", instruction->n);
				break;

			case 0x43:
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,E", instruction->n);
				break;

			case 0x44:
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,H", instruction->n);
				break;

			case 0x45:
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,L", instruction->n);
				break;

			case 0x46:
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if (opcode == 0x66) {
					int i = 0;
				}
				if(get_instruction_name) sprintf(instruction->disassembled_name, "BIT %d,(HL)", instruction->n);
				break;
		}
	}
	else if (prefix == 0xCB && (opcode >= 0x80 && opcode <= 0xBF)) {
		instruction->execute_function = &gbz80_cpu_bitw_res_b_r;
		instruction->cycles = 8;
		instruction->n = (opcode >> 3) & 0x7;

		switch (opcode & 0xC7) {
			case 0x87:
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,A", instruction->n);
				break;

			case 0x80:
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,B", instruction->n);
				break;

			case 0x81:
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,C", instruction->n);
				break;

			case 0x82:
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,D", instruction->n);
				break;

			case 0x83:
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,E", instruction->n);
				break;

			case 0x84:
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,H", instruction->n);
				break;

			case 0x85:
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,L", instruction->n);
				break;

			case 0x86:
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RES %d,(HL)", instruction->n);
				break;
		}
	}
	else if (prefix == 0xCB && (opcode >= 0xC0 && opcode <= 0xFF)) {
		instruction->execute_function = &gbz80_cpu_bitw_set_b_r;
		instruction->cycles = 8;
		instruction->n = (opcode >> 3) & 0x7;

		switch (opcode & 0xC7) {
			case 0xC7:
				instruction->right_r = GBZ80_REGISTER_A;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,A", instruction->n);
				break;

			case 0xC0:
				instruction->right_r = GBZ80_REGISTER_B;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,B", instruction->n);
				break;

			case 0xC1:
				instruction->right_r = GBZ80_REGISTER_C;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,C", instruction->n);
				break;

			case 0xC2:
				instruction->right_r = GBZ80_REGISTER_D;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,D", instruction->n);
				break;

			case 0xC3:
				instruction->right_r = GBZ80_REGISTER_E;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,E", instruction->n);
				break;

			case 0xC4:
				instruction->right_r = GBZ80_REGISTER_H;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,H", instruction->n);
				break;

			case 0xC5:
				instruction->right_r = GBZ80_REGISTER_L;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,L", instruction->n);
				break;

			case 0xC6:
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 16;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "SET %d,(HL)", instruction->n);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xC3)) {
		instruction->execute_function = &gbz80_cpu_jumps_jp_nn;
		instruction->cycles = 12;
		instruction->nn = gbz80_memory_read16(cpu->instance,cpu->registers.PC);
		if(get_instruction_name) sprintf(instruction->disassembled_name, "JP $%04X", instruction->nn);
		cpu->registers.PC += 2;
	}
	else if (prefix == 0x00 && (opcode == 0xC2 || opcode == 0xCA || opcode == 0xD2 || opcode == 0xDA)) {
		instruction->cycles = 12;
		instruction->nn = gbz80_memory_read16(cpu->instance, cpu->registers.PC);
		cpu->registers.PC += 2;

		switch (opcode) {
			case 0xC2:
				instruction->execute_function = &gbz80_cpu_jumps_jpnz_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JP NZ,$%04X", instruction->nn);
				break;

			case 0xCA:
				instruction->execute_function = &gbz80_cpu_jumps_jpz_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JP Z,$%04X", instruction->nn);
				break;

			case 0xD2:
				instruction->execute_function = &gbz80_cpu_jumps_jpnc_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JP NC,$%04X", instruction->nn);
				break;

			case 0xDA:
				instruction->execute_function = &gbz80_cpu_jumps_jpc_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JP C,$%04X", instruction->nn);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xE9)) {
		instruction->cycles = 4;
		instruction->execute_function = &gbz80_cpu_jumps_jp_hl;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "JP (HL)");
	}
	else if (prefix == 0x00 && (opcode == 0x18)) {
		instruction->cycles = 8;
		instruction->d = (int8_t)gbz80_memory_read8(cpu->instance,cpu->registers.PC++);
		instruction->execute_function = &gbz80_cpu_jumps_jr_d;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "JR $%04X", instruction->address + instruction->d + 2);
	}
	else if (prefix == 0x00 && (opcode == 0x20 || opcode == 0x28 || opcode == 0x30 || opcode == 0x38)) {
		instruction->cycles = 8;
		instruction->d = (int8_t)gbz80_memory_read8(cpu->instance, cpu->registers.PC++);

		switch (opcode) {
			case 0x20:
				instruction->execute_function = &gbz80_cpu_jumps_jrnz_d;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JR NZ,$%04X", instruction->address + instruction->d + 2);
				break;

			case 0x28:
				instruction->execute_function = &gbz80_cpu_jumps_jrz_d;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JR Z,$%04X", instruction->address + instruction->d + 2);
				break;

			case 0x30:
				instruction->execute_function = &gbz80_cpu_jumps_jrnc_d;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JR NC,$%04X", instruction->address + instruction->d + 2);
				break;

			case 0x38:
				instruction->execute_function = &gbz80_cpu_jumps_jrc_d;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "JR C,$%04X", instruction->address + instruction->d + 2);
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xCD)) {
		instruction->cycles = 12;
		instruction->nn = gbz80_memory_read16(cpu->instance, cpu->registers.PC);
		instruction->execute_function = &gbz80_cpu_calls_call_nn;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "CALL $%04X", instruction->nn);
		cpu->registers.PC += 2;
	}
	else if (prefix == 0x00 && (opcode == 0xC4 || opcode == 0xCC || opcode == 0xD4 || opcode == 0xDC)) {
		instruction->cycles = 12;
		instruction->nn = gbz80_memory_read16(cpu->instance, cpu->registers.PC);

		switch (opcode) {
			case 0xC4:
				instruction->execute_function = &gbz80_cpu_calls_callnz_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CALL NZ,$%04X", instruction->nn);
				break;

			case 0xCC:
				instruction->execute_function = &gbz80_cpu_calls_callz_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CALL Z,$%04X", instruction->nn);
				break;

			case 0xD4:
				instruction->execute_function = &gbz80_cpu_calls_callnc_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CALL NC,$%04X", instruction->nn);
				break;

			case 0xDC:
				instruction->execute_function = &gbz80_cpu_calls_callc_nn;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "CALL C,$%04X", instruction->nn);
				break;
		}

		cpu->registers.PC += 2;
	}
	else if (prefix == 0x00 && (opcode == 0xC7 || opcode == 0xCF || opcode == 0xD7 || opcode == 0xDF || opcode == 0xE7 || opcode == 0xEF || opcode == 0xF7 || opcode == 0xFF)) {
		instruction->cycles = 32;
		instruction->execute_function = &gbz80_cpu_rsts_rst_n;

		switch (opcode) {
			case 0xC7:
				instruction->n = 0x00;
				break;

			case 0xCF:
				instruction->n = 0x08;
				break;

			case 0xD7:
				instruction->n = 0x10;
				break;

			case 0xDF:
				instruction->n = 0x18;
				break;

			case 0xE7:
				instruction->n = 0x20;
				break;

			case 0xEF:
				instruction->n = 0x28;
				break;

			case 0xF7:
				instruction->n = 0x30;
				break;

			case 0xFF:
				instruction->n = 0x38;
				break;
		}

		if(get_instruction_name) sprintf(instruction->disassembled_name, "RST $%02X", instruction->n);
	}
	else if (prefix == 0x00 && (opcode == 0xC9)) {
		instruction->cycles = 8;
		instruction->execute_function = &gbz80_cpu_rtrns_ret;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "RET");
	}
	else if (prefix == 0x00 && (opcode == 0xC0 || opcode == 0xC8 || opcode == 0xD0 || opcode == 0xD8)) {
		instruction->cycles = 8;

		switch (opcode) {
			case 0xC0:
				instruction->execute_function = &gbz80_cpu_rtrns_retnz;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RET NZ");
				break;

			case 0xC8:
				instruction->execute_function = &gbz80_cpu_rtrns_retz;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RET Z");
				break;

			case 0xD0:
				instruction->execute_function = &gbz80_cpu_rtrns_retnc;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RET NC");
				break;

			case 0xD8:
				instruction->execute_function = &gbz80_cpu_rtrns_retc;
				if(get_instruction_name) sprintf(instruction->disassembled_name, "RET C");
				break;
		}
	}
	else if (prefix == 0x00 && (opcode == 0xD9)) {
		instruction->cycles = 8;
		instruction->execute_function = &gbz80_cpu_rtrns_reti;
		if(get_instruction_name) sprintf(instruction->disassembled_name, "RETI");
	}
}

size_t gbz80_cpu_execute(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	if (instruction->execute_function != NULL) {
		instruction->execute_function(cpu, instruction);
		return instruction->cycles;
	}
	else {
		return 1;
	}
}

void utility_set_flags(gbz80_cpu_t* cpu, uint8_t z, uint8_t n, uint8_t h, uint8_t c) {
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_ZERO, z);
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_N, n);
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_H, h);
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_C, c);
}

void gbz80_cpu_load8_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	gbz80_cpu_set_register8(cpu, instruction->left_r, instruction->n);
}

void gbz80_cpu_load8_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->right_r);
	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_load8_n_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->right_r);
	gbz80_memory_write8(cpu->instance, instruction->nn, val);
}

void gbz80_cpu_load8_a_hl_dec(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t val = gbz80_memory_read8(cpu->instance, hl);
	gbz80_cpu_set_register8(cpu, GBZ80_REGISTER_A, val);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, hl - 1);
}

void gbz80_cpu_load8_hl_dec_a(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t a = gbz80_cpu_get_register8(cpu, GBZ80_REGISTER_A);
	gbz80_memory_write8(cpu->instance, hl, a);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, hl - 1);
}

void gbz80_cpu_load8_a_hl_inc(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t val = gbz80_memory_read8(cpu->instance, hl);
	gbz80_cpu_set_register8(cpu, GBZ80_REGISTER_A, val);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, hl + 1);
}

void gbz80_cpu_load8_hl_inc_a(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t a = gbz80_cpu_get_register8(cpu, GBZ80_REGISTER_A);
	gbz80_memory_write8(cpu->instance, hl, a);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, hl + 1);
}

void gbz80_cpu_load16_r_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	gbz80_cpu_set_register16(cpu, instruction->left_r, instruction->nn);
}

void gbz80_cpu_load16_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t val = gbz80_cpu_get_register16(cpu, instruction->right_r);
	gbz80_cpu_set_register16(cpu, instruction->left_r, val);
}

void gbz80_cpu_load16_hl_sp_plus_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	int16_t val = sp + instruction->d;

	utility_set_flags(cpu, 0, 0, (sp & 0xF) + (instruction->d & 0xF) > 0xF, (sp & 0xFF) + (instruction->d & 0xFF) > 0xFF);

	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, (uint16_t)val);
}

void gbz80_cpu_load16_nn_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction){
	uint16_t val = gbz80_cpu_get_register16(cpu, instruction->right_r);
	gbz80_memory_write16(cpu->instance, instruction->nn, val);
}

void gbz80_cpu_load16_push(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	uint16_t nn = gbz80_cpu_get_register16(cpu, instruction->right_r);

	gbz80_memory_write16(cpu->instance, sp - 2, nn);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp - 2);
}

void gbz80_cpu_load16_pop(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	uint16_t nn = gbz80_memory_read16(cpu->instance, sp);

	if (instruction->left_r == GBZ80_REGISTER_AF) {
		nn &= 0xFFF0;
	}

	gbz80_cpu_set_register16(cpu, instruction->left_r, nn);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp + 2);
}

void gbz80_cpu_alu8_add_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint16_t val = lhs + rhs;
	
	utility_set_flags(cpu, ((uint8_t)val) == 0, 0, (((lhs & 0xf) + (rhs & 0xf)) & 0x10) == 0x10, val > 0xFF);
	
	gbz80_cpu_set_register8(cpu, instruction->left_r, (uint8_t)val);
}

void gbz80_cpu_alu8_add_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint16_t val = lhs + rhs;

	utility_set_flags(cpu, ((uint8_t)val) == 0, 0, (((lhs & 0xf) + (rhs & 0xf)) & 0x10) == 0x10, val > 0xFF);

	gbz80_cpu_set_register8(cpu, instruction->left_r, (uint8_t)val);
}

void gbz80_cpu_alu8_adc_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint16_t val = lhs + rhs + c;

	utility_set_flags(cpu, ((uint8_t)val) == 0, 0, (((lhs & 0xf) + (rhs & 0xf) + (c & 0xf)) & 0x10) == 0x10, val > 0xFF);

	gbz80_cpu_set_register8(cpu, instruction->left_r, (uint8_t)val);
}

void gbz80_cpu_alu8_adc_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint16_t val = lhs + rhs + c;

	utility_set_flags(cpu, ((uint8_t)val) == 0, 0, (((lhs & 0xf) + (rhs & 0xf) + (c & 0xf)) & 0x10) == 0x10, val > 0xFF);

	gbz80_cpu_set_register8(cpu, instruction->left_r, (uint8_t)val);
}

void gbz80_cpu_alu8_sub_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	int16_t val = lhs - rhs;

	utility_set_flags(cpu, ((uint8_t)val) == 0, 1, (((lhs & 0xf) - (rhs & 0xf)) & 0x10) == 0x10, val < 0x0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, lhs - rhs);
}

void gbz80_cpu_alu8_sub_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	int16_t val = lhs - rhs;

	utility_set_flags(cpu, ((uint8_t)val) == 0, 1, (((lhs & 0xf) - (rhs & 0xf)) & 0x10) == 0x10, val < 0x0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, (uint8_t)val);
}

void gbz80_cpu_alu8_sbc_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	int16_t val = lhs - (rhs + c);

	utility_set_flags(cpu, ((uint8_t)val) == 0, 1, (((lhs & 0xf) - (rhs & 0xf) - (c & 0xf)) & 0x10) == 0x10, val < 0x0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, (uint8_t)val);
}

void gbz80_cpu_alu8_sbc_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	int16_t val = lhs - (rhs + c);

	utility_set_flags(cpu, ((uint8_t)val) == 0, 1, (((lhs & 0xf) - (rhs & 0xf) - (c & 0xf)) & 0x10) == 0x10, val < 0x0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, (uint8_t)val);
}

void gbz80_cpu_alu8_and_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t val = lhs & rhs;

	utility_set_flags(cpu, val == 0, 0, 1, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_and_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t val = lhs & rhs;

	utility_set_flags(cpu, val == 0, 0, 1, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_or_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t val = lhs | rhs;

	utility_set_flags(cpu, val == 0, 0, 0, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_or_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t val = lhs | rhs;

	utility_set_flags(cpu, val == 0, 0, 0, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_xor_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t val = lhs ^ rhs;

	utility_set_flags(cpu, val == 0, 0, 0, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_xor_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t val = lhs ^ rhs;

	utility_set_flags(cpu, val == 0, 0, 0, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_cp_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t val = lhs - rhs;

	utility_set_flags(cpu, lhs == rhs, 1, (((lhs & 0xf) - (rhs & 0xf)) & 0x10) == 0x10, lhs < rhs);
}

void gbz80_cpu_alu8_cp_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t val = lhs - rhs;

	utility_set_flags(cpu, lhs == rhs, 1, (((lhs & 0xf) - (rhs & 0xf)) & 0x10) == 0x10, lhs < rhs);
}

void gbz80_cpu_alu8_inc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t val = lhs + 1;
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);

	utility_set_flags(cpu, val == 0, 0, (((lhs & 0xf) + (1 & 0xf)) & 0x10) == 0x10, c);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_dec_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t val = lhs - 1;
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);

	utility_set_flags(cpu, val == 0, 1, (((lhs & 0xf) - (1 & 0xf)) & 0x10) == 0x10, c);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu16_add_hl_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t lhs = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint16_t rhs = gbz80_cpu_get_register16(cpu, instruction->right_r);
	uint32_t val = lhs + rhs;
	uint8_t z = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO);

	utility_set_flags(cpu, z, 0, (((lhs & 0xFFF) + (rhs & 0xFFF)) & 0x1000) == 0x1000, (val & 0x10000) == 0x10000);

	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, (uint16_t)val);
}

void gbz80_cpu_alu16_add_sp_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t lhs = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	int16_t rhs = instruction->d;
	uint16_t val = lhs + rhs;

	utility_set_flags(cpu, 0, 0, (lhs & 0xF) + (rhs & 0xF) > 0xF, (lhs & 0xFF) + (rhs & 0xFF) > 0xFF);

	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, val);
}

void gbz80_cpu_alu16_inc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint16_t lhs = gbz80_cpu_get_register16(cpu, instruction->left_r);
	uint16_t val = lhs + 1;
	gbz80_cpu_set_register16(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu16_dec_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint16_t lhs = gbz80_cpu_get_register16(cpu, instruction->left_r);
	uint16_t val = lhs - 1;
	gbz80_cpu_set_register16(cpu, instruction->left_r, val);
}

void gbz80_cpu_misc_swap(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	val = ((val & 0xF) << 4) | ((val & 0xF0) >> 4);
	utility_set_flags(cpu, val == 0, 0, 0, 0);
	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_misc_daa(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	int16_t val = gbz80_cpu_get_register8(cpu, GBZ80_REGISTER_A);
	uint8_t h = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_H);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t n = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_N);
	
	if (n == 0) {
		if (h || (val & 0x0F) > 0x09) {
			val += 0x06;
		}
		if (c || val > 0x9F) {
			val += 0x60;
		}
	}
	else {
		if (h) {
			val -= 0x6;
			val &= 0xFF;
		}
		if (c) {
			val -= 0x60;
		}
	}

	
	utility_set_flags(cpu, (val & 0xFF) == 0, n, 0, (val & 0x100) == 0x100 ? 1 : c);

	gbz80_cpu_set_register8(cpu, GBZ80_REGISTER_A, (uint8_t)(val & 0xFF));
}

void gbz80_cpu_misc_cpl(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, GBZ80_REGISTER_A);
	uint8_t z = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	utility_set_flags(cpu, z, 1, 1, c);
	val = ~val;
	gbz80_cpu_set_register8(cpu, GBZ80_REGISTER_A, val);
}

void gbz80_cpu_misc_ccf(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t z = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO);
	utility_set_flags(cpu, z, 0, 0, (~c) & 0x1);
}

void gbz80_cpu_misc_scf(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t z = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO);
	utility_set_flags(cpu, z, 0, 0, 1);
}

void gbz80_cpu_misc_nop(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
}

void gbz80_cpu_misc_halt(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	//TODO
}

void gbz80_cpu_misc_stop(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	//TODO
}

void gbz80_cpu_misc_di(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	cpu->ime = 0;
	cpu->ime_ready = 0;
}

void gbz80_cpu_misc_ei(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	cpu->ime_ready = 1;
}

void gbz80_cpu_rtsh_rl_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t result = (val << 1) | c;

	utility_set_flags(cpu, result == 0, 0, 0, (val >> 7) & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_rlc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t result = (val << 1) | (val >> 7);

	utility_set_flags(cpu, result == 0, 0, 0, (val >> 7) & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_rla(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t result = (val << 1) | c;

	utility_set_flags(cpu, 0, 0, 0, (val >> 7) & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_rlca(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t result = (val << 1) | (val >> 7);

	utility_set_flags(cpu, 0, 0, 0, (val >> 7) & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}


void gbz80_cpu_rtsh_rr_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t result = (val >> 1) | (c << 7);

	utility_set_flags(cpu, result == 0, 0, 0, val & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_rrc_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t result = (val >> 1) | (val << 7);

	utility_set_flags(cpu, result == 0, 0, 0, val & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_rra(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t result = (val >> 1) | (c << 7);

	utility_set_flags(cpu, 0, 0, 0, val & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_rrca(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t result = (val >> 1) | (val << 7);

	utility_set_flags(cpu, 0, 0, 0, val & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_sla_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t result = val << 1;

	utility_set_flags(cpu, result == 0, 0, 0, (val >> 7) & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_sra_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t result = val >> 1 | (val & 0x80);

	utility_set_flags(cpu, result == 0, 0, 0, val & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_rtsh_srl_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t result = val >> 1;

	utility_set_flags(cpu, result == 0, 0, 0, val & 0x1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, result);
}

void gbz80_cpu_bitw_bit_b_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t bit = common_get8_bit(val, instruction->n);

	utility_set_flags(cpu, bit == 0, 0, 1, c);
}

void gbz80_cpu_bitw_set_b_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->right_r);
	common_set8_bit(&val, instruction->n);
	gbz80_cpu_set_register8(cpu, instruction->right_r, val);
}

void gbz80_cpu_bitw_res_b_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->right_r);
	common_reset8_bit(&val, instruction->n);
	gbz80_cpu_set_register8(cpu, instruction->right_r, val);
}

void gbz80_cpu_jumps_jp_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	cpu->registers.PC = instruction->nn;
}

void gbz80_cpu_jumps_jpnz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 0) {
		gbz80_cpu_jumps_jp_nn(cpu, instruction);
	}
}

void gbz80_cpu_jumps_jpz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 1) {
		gbz80_cpu_jumps_jp_nn(cpu, instruction);
	}
}

void gbz80_cpu_jumps_jpnc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 0) {
		gbz80_cpu_jumps_jp_nn(cpu, instruction);
	}
}

void gbz80_cpu_jumps_jpc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 1) {
		gbz80_cpu_jumps_jp_nn(cpu, instruction);
	}
}

void gbz80_cpu_jumps_jp_hl(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	cpu->registers.PC = gbz80_cpu_get_register16(cpu,GBZ80_REGISTER_HL);
}

void gbz80_cpu_jumps_jr_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	cpu->registers.PC = instruction->address + instruction->d + 2;
}

void gbz80_cpu_jumps_jrnz_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 0) {
		gbz80_cpu_jumps_jr_d(cpu, instruction);
	}
}

void gbz80_cpu_jumps_jrz_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 1) {
		gbz80_cpu_jumps_jr_d(cpu, instruction);
	}
}

void gbz80_cpu_jumps_jrnc_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 0) {
		gbz80_cpu_jumps_jr_d(cpu, instruction);
	}
}

void gbz80_cpu_jumps_jrc_d(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 1) {
		gbz80_cpu_jumps_jr_d(cpu, instruction);
	}
}

void gbz80_cpu_calls_call_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	gbz80_memory_write16(cpu->instance, sp - 2, cpu->registers.PC);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp - 2);
	cpu->registers.PC = instruction->nn;
}

void gbz80_cpu_calls_callnz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 0) {
		gbz80_cpu_calls_call_nn(cpu, instruction);
	}
}

void gbz80_cpu_calls_callz_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 1) {
		gbz80_cpu_calls_call_nn(cpu, instruction);
	}
}

void gbz80_cpu_calls_callnc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 0) {
		gbz80_cpu_calls_call_nn(cpu, instruction);
	}
}

void gbz80_cpu_calls_callc_nn(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 1) {
		gbz80_cpu_calls_call_nn(cpu, instruction);
	}
}

void gbz80_cpu_rsts_rst_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	gbz80_memory_write16(cpu->instance, sp - 2, cpu->registers.PC);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp - 2);
	cpu->registers.PC = 0x0000 + instruction->n;
}

void gbz80_cpu_rtrns_ret(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	uint16_t nn = gbz80_memory_read16(cpu->instance, sp);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp + 2);
	cpu->registers.PC = nn;
}

void gbz80_cpu_rtrns_retnz(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 0) {
		gbz80_cpu_rtrns_ret(cpu, instruction);
	}
}

void gbz80_cpu_rtrns_retz(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_ZERO) == 1) {
		gbz80_cpu_rtrns_ret(cpu, instruction);
	}
}

void gbz80_cpu_rtrns_retnc(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 0) {
		gbz80_cpu_rtrns_ret(cpu, instruction);
	}
}

void gbz80_cpu_rtrns_retc(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	if (gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C) == 1) {
		gbz80_cpu_rtrns_ret(cpu, instruction);
	}
}

void gbz80_cpu_rtrns_reti(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	gbz80_cpu_rtrns_ret(cpu, instruction);
	gbz80_cpu_misc_ei(cpu, instruction);
}