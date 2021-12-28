#include "gbz80_cpu.h"

void gbz80_cpu_init(gbz80_cpu_t* cpu, uint8_t* memory, size_t memory_size) {
	memset(cpu, 0, sizeof(gbz80_cpu_t));
	cpu->memory = memory;
	cpu->memory_size = memory_size;
	cpu->registers.PC = 0x0100;
	cpu->registers.SP = 0xFFFE;
}

void gbz80_cpu_set_flag(gbz80_cpu_t* cpu, gbz80_flag_t flag, uint8_t val) {
	if (val != 0 && val != 1) {
		assert(0 && "Flag has been set to a suspicious value");
		return;
	}

	cpu->registers.flags ^= val << (uint8_t)flag;
}

uint8_t gbz80_cpu_get_flag(gbz80_cpu_t* cpu, gbz80_flag_t flag){
	return (cpu->registers.flags >> (uint8_t)flag) & 0x1;
}

uint8_t gbz80_cpu_memory_read8(gbz80_cpu_t* cpu, uint16_t address) {
	return cpu->memory[address];
}

void gbz80_cpu_memory_write8(gbz80_cpu_t* cpu, uint16_t address, uint8_t val) {
	cpu->memory[address] = val;
}

uint16_t gbz80_cpu_memory_read16(gbz80_cpu_t* cpu, uint16_t address) {
	return (uint16_t)gbz80_cpu_memory_read8(cpu,address + 1) << 8 | gbz80_cpu_memory_read8(cpu, address);
}

void gbz80_cpu_memory_write16(gbz80_cpu_t* cpu, uint16_t address, uint16_t val) {
	gbz80_cpu_memory_write8(cpu, address, (uint8_t)(val & 0xFF));
	gbz80_cpu_memory_write8(cpu, address + 1, (uint8_t)((val >> 8) & 0xFF));
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
			gbz80_cpu_memory_write8(cpu, cpu->registers.AF, val);
			break;

		case GBZ80_REGISTER_BC:
			gbz80_cpu_memory_write8(cpu, cpu->registers.BC, val);
			break;

		case GBZ80_REGISTER_DE:
			gbz80_cpu_memory_write8(cpu, cpu->registers.DE, val);
			break;

		case GBZ80_REGISTER_HL:
			gbz80_cpu_memory_write8(cpu, cpu->registers.HL, val);
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
		return gbz80_cpu_memory_read8(cpu, cpu->registers.AF);

	case GBZ80_REGISTER_BC:
		return gbz80_cpu_memory_read8(cpu, cpu->registers.BC);

	case GBZ80_REGISTER_DE:
		return gbz80_cpu_memory_read8(cpu, cpu->registers.DE);

	case GBZ80_REGISTER_HL:
		return gbz80_cpu_memory_read8(cpu, cpu->registers.HL);
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

size_t gbz80_cpu_step(gbz80_cpu_t* cpu)
{
	size_t clock_cycles;
	gbz80_instruction_t instruction;
	memset(&instruction, 0, sizeof(gbz80_instruction_t));

	gbz80_cpu_fetch(cpu, &instruction);
	gbz80_cpu_decode(cpu, &instruction);
	clock_cycles = gbz80_cpu_execute(cpu, &instruction);

	return clock_cycles;
}

void gbz80_cpu_fetch(gbz80_cpu_t* cpu, gbz80_instruction_t* out_instruction)
{
	uint8_t opcode = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
	if (opcode == 0xCB) {
		out_instruction->prefix = opcode;
		opcode = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
	}
	out_instruction->opcode = opcode;
}

void gbz80_cpu_decode(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {

	uint8_t opcode = instruction->opcode;
	if (opcode == 0x06 || opcode == 0x0E || opcode == 0x16 || opcode == 0x1E || opcode == 0x26 || opcode == 0x2E) {
		instruction->n = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
		instruction->execute_function = &gbz80_cpu_load8_r_n;
		instruction->cycles = 8;

		switch (instruction->opcode)
		{
			case 0x06:
				sprintf(instruction->disassembled_name, "LD B,$%04X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_B;
				break;

			case 0x0E:
				sprintf(instruction->disassembled_name, "LD C,$%04X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_C;
				break;

			case 0x16:
				sprintf(instruction->disassembled_name, "LD D,$%04X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_D;
				break;

			case 0x1E:
				sprintf(instruction->disassembled_name, "LD E,$%04X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_E;
				break;

			case 0x26:
				sprintf(instruction->disassembled_name, "LD H$%04X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_H;
				break;

			case 0x2E:
				sprintf(instruction->disassembled_name, "LD L,$%04X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_L;
				break;
		}
	}
	else if (	opcode == 0x7F || opcode == 0x78 || opcode == 0x79 || opcode == 0x7A || opcode == 0x7B || opcode == 0x7C || opcode == 0x7D || opcode == 0x7E || opcode == 0x40 || opcode == 0x41 || 
				opcode == 0x42 || opcode == 0x43 || opcode == 0x44 || opcode == 0x45 || opcode == 0x46 || opcode == 0x48 || opcode == 0x49 || opcode == 0x4A || opcode == 0x4B || opcode == 0x4C || 
				opcode == 0x4D || opcode == 0x4E || opcode == 0x50 || opcode == 0x51 || opcode == 0x52 || opcode == 0x53 || opcode == 0x54 || opcode == 0x55 || opcode == 0x56 || opcode == 0x58 || 
				opcode == 0x59 || opcode == 0x5A || opcode == 0x5B || opcode == 0x5C || opcode == 0x5D || opcode == 0x5E || opcode == 0x60 || opcode == 0x61 || opcode == 0x62 || opcode == 0x63 || 
				opcode == 0x64 || opcode == 0x65 || opcode == 0x66 || opcode == 0x68 || opcode == 0x69 || opcode == 0x6A || opcode == 0x6B || opcode == 0x6C || opcode == 0x6D || opcode == 0x6E || 
				opcode == 0x70 || opcode == 0x71 || opcode == 0x72 || opcode == 0x73 || opcode == 0x74 || opcode == 0x75 || opcode == 0x36) {
			instruction->execute_function = &gbz80_cpu_load8_r_r;
			instruction->cycles = 4;

			switch (opcode) {
				case 0x7F:
					sprintf(instruction->disassembled_name, "LD A,A");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_A;
					break;

				case 0x78:
					sprintf(instruction->disassembled_name, "LD A,B");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x79:
					sprintf(instruction->disassembled_name, "LD A,C");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x7A:
					sprintf(instruction->disassembled_name, "LD A,D");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x7B:
					sprintf(instruction->disassembled_name, "LD A,E");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x7C:
					sprintf(instruction->disassembled_name, "LD A,H");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x7D:
					sprintf(instruction->disassembled_name, "LD A,L");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x7E:
					sprintf(instruction->disassembled_name, "LD A,(HL)");
					instruction->left_r = GBZ80_REGISTER_A;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x40:
					sprintf(instruction->disassembled_name, "LD B,B");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x41:
					sprintf(instruction->disassembled_name, "LD B,C");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x42:
					sprintf(instruction->disassembled_name, "LD B,D");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x43:
					sprintf(instruction->disassembled_name, "LD B,E");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x44:
					sprintf(instruction->disassembled_name, "LD B,H");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x45:
					sprintf(instruction->disassembled_name, "LD B,L");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x46:
					sprintf(instruction->disassembled_name, "LD B,(HL)");
					instruction->left_r = GBZ80_REGISTER_B;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x48:
					sprintf(instruction->disassembled_name, "LD C,B");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x49:
					sprintf(instruction->disassembled_name, "LD C,C");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x4A:
					sprintf(instruction->disassembled_name, "LD C,D");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x4B:
					sprintf(instruction->disassembled_name, "LD C,E");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x4C:
					sprintf(instruction->disassembled_name, "LD C,H");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x4D:
					sprintf(instruction->disassembled_name, "LD C,L");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x4E:
					sprintf(instruction->disassembled_name, "LD C,(HL)");
					instruction->left_r = GBZ80_REGISTER_C;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x50:
					sprintf(instruction->disassembled_name, "LD D,B");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x51:
					sprintf(instruction->disassembled_name, "LD D,C");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x52:
					sprintf(instruction->disassembled_name, "LD D,D");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x53:
					sprintf(instruction->disassembled_name, "LD D,E");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x54:
					sprintf(instruction->disassembled_name, "LD D,H");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x55:
					sprintf(instruction->disassembled_name, "LD D,L");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x56:
					sprintf(instruction->disassembled_name, "LD D,(HL)");
					instruction->left_r = GBZ80_REGISTER_D;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x58:
					sprintf(instruction->disassembled_name, "LD E,B");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x59:
					sprintf(instruction->disassembled_name, "LD E,C");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x5A:
					sprintf(instruction->disassembled_name, "LD E,D");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x5B:
					sprintf(instruction->disassembled_name, "LD E,E");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x5C:
					sprintf(instruction->disassembled_name, "LD E,H");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x5D:
					sprintf(instruction->disassembled_name, "LD E,L");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x5E:
					sprintf(instruction->disassembled_name, "LD E,(HL)");
					instruction->left_r = GBZ80_REGISTER_E;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x60:
					sprintf(instruction->disassembled_name, "LD H,B");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x61:
					sprintf(instruction->disassembled_name, "LD H,C");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x62:
					sprintf(instruction->disassembled_name, "LD H,D");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x63:
					sprintf(instruction->disassembled_name, "LD H,E");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x64:
					sprintf(instruction->disassembled_name, "LD H,H");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x65:
					sprintf(instruction->disassembled_name, "LD H,L");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x66:
					sprintf(instruction->disassembled_name, "LD H,(HL)");
					instruction->left_r = GBZ80_REGISTER_H;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x68:
					sprintf(instruction->disassembled_name, "LD L,B");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_B;
					break;

				case 0x69:
					sprintf(instruction->disassembled_name, "LD L,C");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_C;
					break;

				case 0x6A:
					sprintf(instruction->disassembled_name, "LD L,D");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_D;
					break;

				case 0x6B:
					sprintf(instruction->disassembled_name, "LD L,E");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_E;
					break;

				case 0x6C:
					sprintf(instruction->disassembled_name, "LD L,H");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_H;
					break;

				case 0x6D:
					sprintf(instruction->disassembled_name, "LD L,L");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_L;
					break;

				case 0x6E:
					sprintf(instruction->disassembled_name, "LD L,(HL)");
					instruction->left_r = GBZ80_REGISTER_L;
					instruction->right_r = GBZ80_REGISTER_HL;
					instruction->cycles = 8;
					break;

				case 0x70:
					sprintf(instruction->disassembled_name, "LD (HL),B");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_B;
					instruction->cycles = 8;
					break;

				case 0x71:
					sprintf(instruction->disassembled_name, "LD (HL),C");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_C;
					instruction->cycles = 8;
					break;

				case 0x72:
					sprintf(instruction->disassembled_name, "LD (HL),D");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_D;
					instruction->cycles = 8;
					break;

				case 0x73:
					sprintf(instruction->disassembled_name, "LD (HL),E");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_E;
					instruction->cycles = 8;
					break;

				case 0x74:
					sprintf(instruction->disassembled_name, "LD (HL),H");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_H;
					instruction->cycles = 8;
					break;

				case 0x75:
					sprintf(instruction->disassembled_name, "LD (HL),L");
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->right_r = GBZ80_REGISTER_L;
					instruction->cycles = 8;
					break;

				case 0x36:
					instruction->n = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
					sprintf(instruction->disassembled_name, "LD (HL),$%04X", instruction->n);
					instruction->execute_function = &gbz80_cpu_load8_r_n;
					instruction->left_r = GBZ80_REGISTER_HL;
					instruction->cycles = 12;
					break;

			}
	} 
	else if (opcode == 0x0A || opcode == 0x1A || opcode == 0x7E || opcode == 0xFA || opcode == 0x3E) {
		instruction->execute_function = &gbz80_cpu_load8_r_r;
		instruction->cycles = 8;
		switch (opcode)
		{
			case 0x0A:
				sprintf(instruction->disassembled_name, "LD A,(BC)");
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_BC;
				break;

			case 0x1A:
				sprintf(instruction->disassembled_name, "LD A,(DE)");
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_DE;
				break;

			case 0x7E:
				sprintf(instruction->disassembled_name, "LD A,(HL)");
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				break;

			case 0xFA: {
				uint16_t nn = gbz80_cpu_memory_read16(cpu, cpu->registers.PC);
				instruction->n = gbz80_cpu_memory_read8(cpu, nn);
				sprintf(instruction->disassembled_name, "LD A,($%04X)", nn);
				instruction->left_r= GBZ80_REGISTER_A;
				instruction->execute_function = &gbz80_cpu_load8_r_n;
				instruction->cycles = 16;
				cpu->registers.PC += 2;
				break;
			}

			case 0x3E:
				instruction->n = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
				sprintf(instruction->disassembled_name, "LD A,$%04X", instruction->n);
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->execute_function = &gbz80_cpu_load8_r_n;
				break;
		}
	}
	else if (opcode == 0x02 || opcode == 0x12 || opcode == 0x77 || opcode == 0xEA) {
		instruction->execute_function = &gbz80_cpu_load8_r_r;
		instruction->cycles = 8;
		switch (opcode)
		{
			case 0x02:
				sprintf(instruction->disassembled_name, "LD (BC),A");
				instruction->left_r = GBZ80_REGISTER_BC;
				instruction->right_r = GBZ80_REGISTER_A;
				break;

			case 0x12:
				sprintf(instruction->disassembled_name, "LD (DE),A");
				instruction->left_r = GBZ80_REGISTER_DE;
				instruction->right_r = GBZ80_REGISTER_A;
				break;

			case 0x77:
				sprintf(instruction->disassembled_name, "LD (HL),A");
				instruction->left_r = GBZ80_REGISTER_HL;
				instruction->right_r = GBZ80_REGISTER_A;
				break;

			case 0xEA: {
				instruction->nn = gbz80_cpu_memory_read8(cpu, cpu->registers.PC);
				sprintf(instruction->disassembled_name, "LD ($%04X),A", instruction->nn);
				instruction->right_r = GBZ80_REGISTER_A;
				instruction->execute_function = &gbz80_cpu_load8_n_r;
				instruction->cycles = 16;
				cpu->registers.PC += 2;
				break;
			}
		}
	}
	else if (opcode == 0xF2) {
		instruction->execute_function = &gbz80_cpu_load8_r_n;
		instruction->cycles = 8;
		instruction->n = gbz80_cpu_memory_read8(cpu, 0xFF00 + cpu->registers.C);
		sprintf(instruction->disassembled_name, "LD A,($FF00+C)");
		instruction->left_r = GBZ80_REGISTER_A;
	}
	else if (opcode == 0xE2) {
		instruction->execute_function = &gbz80_cpu_load8_n_r;
		instruction->cycles = 8;
		instruction->nn = 0xFF00 + cpu->registers.C;
		sprintf(instruction->disassembled_name, "LD ($FF00+C),A");
		instruction->right_r = GBZ80_REGISTER_A;
	}
	else if (opcode == 0x3A) {
		instruction->execute_function = &gbz80_cpu_load8_a_hl_dec;
		instruction->cycles = 8;
		sprintf(instruction->disassembled_name, "LD A,(HL-)");
	}
	else if (opcode == 0x32) {
		instruction->execute_function = &gbz80_cpu_load8_hl_dec_a;
		instruction->cycles = 8;
		sprintf(instruction->disassembled_name, "LD (HL-),A");
	}
	else if (opcode == 0x2A) {
		instruction->execute_function = &gbz80_cpu_load8_a_hl_inc;
		instruction->cycles = 8;
		sprintf(instruction->disassembled_name, "LD A,(HL+)");
	}
	else if (opcode == 0x22) {
		instruction->execute_function = &gbz80_cpu_load8_hl_inc_a;
		instruction->cycles = 8;
		sprintf(instruction->disassembled_name, "LD (HL+),A");
	}
	else if (opcode == 0xE0) {
		instruction->execute_function = &gbz80_cpu_load8_n_r;
		instruction->cycles = 12;
		uint8_t n = gbz80_cpu_memory_read8(cpu,cpu->registers.PC++);
		instruction->nn = 0xFF00 + n;
		sprintf(instruction->disassembled_name, "LD ($FF00+$%04X),A",n);
		instruction->right_r = GBZ80_REGISTER_A;
	}
	else if (opcode == 0xE0) {
		instruction->execute_function = &gbz80_cpu_load8_r_n;
		instruction->cycles = 12;
		uint8_t n = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
		instruction->n = gbz80_cpu_memory_read8(cpu, 0xFF00 + n);
		sprintf(instruction->disassembled_name, "LD A,($FF00+$%04X)", n);
		instruction->right_r = GBZ80_REGISTER_A;
	}
	else if (opcode == 0x01 || opcode == 0x11 || opcode == 0x21 || opcode == 0x31) {
		instruction->execute_function = &gbz80_cpu_load16_r_nn;
		instruction->cycles = 12;
		instruction->nn = gbz80_cpu_memory_read16(cpu, cpu->registers.PC);
		cpu->registers.PC += 2;

		switch (opcode) {
			case 0x01:
				sprintf(instruction->disassembled_name, "LD BC,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_BC;
				break;

			case 0x11:
				sprintf(instruction->disassembled_name, "LD DE,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_DE;
				break;

			case 0x21:
				sprintf(instruction->disassembled_name, "LD HL,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_HL;
				break;

			case 0x31:
				sprintf(instruction->disassembled_name, "LD SP,$%04X", instruction->nn);
				instruction->left_r = GBZ80_REGISTER_SP;
				break;
		}
	}
	else if (opcode == 0xF9) {
		instruction->execute_function = &gbz80_cpu_load16_r_r;
		instruction->cycles = 8;
		sprintf(instruction->disassembled_name, "LD SP,HL");
		instruction->left_r = GBZ80_REGISTER_SP;
		instruction->right_r = GBZ80_REGISTER_HL;
	}
	else if (opcode == 0xF8) {
		instruction->execute_function = &gbz80_cpu_load16_hl_sp_plus_n;
		instruction->cycles = 12;
		instruction->d = (int8_t)gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
		sprintf(instruction->disassembled_name, "LDHL SP,$%04X", instruction->d);
	}
	else if (opcode == 0x08) {
		instruction->execute_function = &gbz80_cpu_load16_nn_r;
		instruction->cycles = 20;
		instruction->nn = gbz80_cpu_memory_read16(cpu, cpu->registers.PC);
		cpu->registers.PC += 2;
		sprintf(instruction->disassembled_name, "LD ($%04X),SP", instruction->nn);
	}
	else if (opcode == 0xF5 || opcode == 0xC5 || opcode == 0xD5 || opcode == 0xE5) {
		instruction->execute_function = &gbz80_cpu_load16_push;
		instruction->cycles = 16;

		switch (opcode) {
			case 0xF5:
				instruction->right_r = GBZ80_REGISTER_AF;
				sprintf(instruction->disassembled_name, "PUSH AF");
				break;

			case 0xC5:
				instruction->right_r = GBZ80_REGISTER_BC;
				sprintf(instruction->disassembled_name, "PUSH BC");
				break;

			case 0xD5:
				instruction->right_r = GBZ80_REGISTER_DE;
				sprintf(instruction->disassembled_name, "PUSH DE");
				break;

			case 0xE5:
				instruction->right_r = GBZ80_REGISTER_HL;
				sprintf(instruction->disassembled_name, "PUSH HL");
				break;
		}
	}
	else if (opcode == 0xF1 || opcode == 0xC1 || opcode == 0xD1 || opcode == 0xE1) {
		instruction->execute_function = &gbz80_cpu_load16_pop;
		instruction->cycles = 12;

		switch (opcode) {
		case 0xF1:
			instruction->left_r = GBZ80_REGISTER_AF;
			sprintf(instruction->disassembled_name, "POP AF");
			break;

		case 0xC1:
			instruction->left_r = GBZ80_REGISTER_BC;
			sprintf(instruction->disassembled_name, "POP BC");
			break;

		case 0xD1:
			instruction->left_r = GBZ80_REGISTER_DE;
			sprintf(instruction->disassembled_name, "POP DE");
			break;

		case 0xE1:
			instruction->left_r = GBZ80_REGISTER_HL;
			sprintf(instruction->disassembled_name, "POP HL");
			break;
		}
	}
	else if (opcode == 0x87 || opcode == 0x80 || opcode == 0x81 || opcode == 0x82 || opcode == 0x83 || opcode == 0x84 || opcode == 0x85 || opcode == 0x86 || opcode == 0xC6) {
		instruction->execute_function = &gbz80_cpu_alu8_add_r_r;
		instruction->cycles = 4;
		
		switch (opcode) {
			case 0x87:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				sprintf(instruction->disassembled_name, "ADD A,A");
				break;

			case 0x80:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				sprintf(instruction->disassembled_name, "ADD A,B");
				break;

			case 0x81:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				sprintf(instruction->disassembled_name, "ADD A,C");
				break;

			case 0x82:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				sprintf(instruction->disassembled_name, "ADD A,D");
				break;

			case 0x83:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				sprintf(instruction->disassembled_name, "ADD A,E");
				break;

			case 0x84:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				sprintf(instruction->disassembled_name, "ADD A,H");
				break;

			case 0x85:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				sprintf(instruction->disassembled_name, "ADD A,L");
				break;

			case 0x86:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				sprintf(instruction->disassembled_name, "ADD A,(HL)");
				break;

			case 0xC6:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_add_r_n;
				instruction->cycles = 8;
				sprintf(instruction->disassembled_name, "ADD A,$%04X", instruction->n);
				break;
			
		}
	}
	else if (opcode == 0x8F || opcode == 0x88 || opcode == 0x89 || opcode == 0x8A || opcode == 0x8B || opcode == 0x8C || opcode == 0x8D || opcode == 0x8E || opcode == 0xCE) {
		instruction->execute_function = &gbz80_cpu_alu8_adc_r_r;
		instruction->cycles = 4;

		switch (opcode) {
			case 0x8F:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_A;
				sprintf(instruction->disassembled_name, "ADC A,A");
				break;

			case 0x88:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_B;
				sprintf(instruction->disassembled_name, "ADC A,B");
				break;

			case 0x89:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_C;
				sprintf(instruction->disassembled_name, "ADC A,C");
				break;

			case 0x8A:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_D;
				sprintf(instruction->disassembled_name, "ADC A,D");
				break;

			case 0x8B:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_E;
				sprintf(instruction->disassembled_name, "ADC A,E");
				break;

			case 0x8C:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_H;
				sprintf(instruction->disassembled_name, "ADC A,H");
				break;

			case 0x8D:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_L;
				sprintf(instruction->disassembled_name, "ADC A,L");
				break;

			case 0x8E:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->right_r = GBZ80_REGISTER_HL;
				instruction->cycles = 8;
				sprintf(instruction->disassembled_name, "ADC A,(HL)");
				break;

			case 0xCE:
				instruction->left_r = GBZ80_REGISTER_A;
				instruction->n = gbz80_cpu_memory_read8(cpu, cpu->registers.PC++);
				instruction->execute_function = &gbz80_cpu_alu8_adc_r_n;
				instruction->cycles = 8;
				sprintf(instruction->disassembled_name, "ADC A,$%04X", instruction->n);
				break;

		}
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

void utility_set_flags(gbz80_cpu_t* cpu, uint16_t result, uint8_t substraction) {
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_ZERO, result == 0);
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_N, substraction);
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_H, result > 0xFF);
	gbz80_cpu_set_flag(cpu, GBZ80_FLAG_C, result > 0xFFFF);
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
	gbz80_cpu_memory_write8(cpu, instruction->nn, val);
}

void gbz80_cpu_load8_a_hl_dec(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t val = gbz80_cpu_memory_read8(cpu, hl);
	gbz80_cpu_set_register8(cpu, GBZ80_REGISTER_A, val);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, hl - 1);
}

void gbz80_cpu_load8_hl_dec_a(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t a = gbz80_cpu_get_register8(cpu, GBZ80_REGISTER_A);
	gbz80_cpu_memory_write8(cpu, hl, a);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, hl - 1);
}

void gbz80_cpu_load8_a_hl_inc(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t val = gbz80_cpu_memory_read8(cpu, hl);
	gbz80_cpu_set_register8(cpu, GBZ80_REGISTER_A, val);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, hl + 1);
}

void gbz80_cpu_load8_hl_inc_a(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t hl = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_HL);
	uint8_t a = gbz80_cpu_get_register8(cpu, GBZ80_REGISTER_A);
	gbz80_cpu_memory_write8(cpu, hl, a);
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
	uint32_t val = sp + instruction->d;

	utility_set_flags(cpu, val, 0);

	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_HL, (uint16_t)val);
}

void gbz80_cpu_load16_nn_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction){
	uint8_t val = gbz80_cpu_get_register8(cpu, instruction->right_r);
	gbz80_cpu_memory_write8(cpu, instruction->nn, val);
}

void gbz80_cpu_load16_push(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	uint16_t nn = gbz80_cpu_get_register16(cpu, instruction->right_r);

	gbz80_cpu_memory_write16(cpu, sp - 2, nn);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp - 2);
}

void gbz80_cpu_alu8_add_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction)
{
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t val = lhs + rhs;

	utility_set_flags(cpu, val, 0);
	
	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_add_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t val = lhs + rhs;

	utility_set_flags(cpu, val, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_adc_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t val = lhs + rhs + c;

	utility_set_flags(cpu, val, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_adc_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t val = lhs + rhs + c;

	utility_set_flags(cpu, val, 0);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_sub_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t val = lhs - rhs;

	utility_set_flags(cpu, val, 1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_sub_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t val = lhs - rhs;

	utility_set_flags(cpu, val, 1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_sbc_r_r(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = gbz80_cpu_get_register8(cpu, instruction->right_r);
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t val = lhs - (rhs + c);

	utility_set_flags(cpu, val, 1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_alu8_sbc_r_n(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint8_t lhs = gbz80_cpu_get_register8(cpu, instruction->left_r);
	uint8_t rhs = instruction->n;
	uint8_t c = gbz80_cpu_get_flag(cpu, GBZ80_FLAG_C);
	uint8_t val = lhs - (rhs + c);

	utility_set_flags(cpu, val, 1);

	gbz80_cpu_set_register8(cpu, instruction->left_r, val);
}

void gbz80_cpu_load16_pop(gbz80_cpu_t* cpu, gbz80_instruction_t* instruction) {
	uint16_t sp = gbz80_cpu_get_register16(cpu, GBZ80_REGISTER_SP);
	uint16_t nn = gbz80_cpu_memory_read16(cpu, sp);

	gbz80_cpu_set_register16(cpu, instruction->left_r, nn);
	gbz80_cpu_set_register16(cpu, GBZ80_REGISTER_SP, sp + 2);
}