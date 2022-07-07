#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

typedef int64_t word;

#define INITIAL_MEMSZ 512

enum mnemonics {
	ADD  = 1,
	MUL  = 2,
	INP  = 3,
	OUT  = 4,
	JNE  = 5,
	JEQ  = 6,
	LT   = 7,
	EQ   = 8,
	INCB = 9,

	HLT  = 99
};

struct computer {
	word* mem;
	int   mem_sz;
	int   prog_sz; // only needed for disasm option
	int   pc;
	int   base; // "relative base"
};

bool read_int(char** pLine, word* x) {
	word num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (*line < '0' || *line > '9')
		return false;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = isneg ? -num : num;
	*pLine = line;
	return true;
}

void computer_double_memsz(struct computer* comp) {
	comp->mem = realloc(comp->mem, 2 * comp->mem_sz * sizeof(word));
	memset(comp->mem + comp->mem_sz, 0, comp->mem_sz * sizeof(word));
	comp->mem_sz *= 2;
}

void computer_set_mem(struct computer* comp, int addr, word val) {
	while (addr >= comp->mem_sz) {
		computer_double_memsz(comp);
		printf("INFO: setting memory size to %d due to write at addres %d\n", comp->mem_sz, addr);
	}
	comp->mem[addr] = val;
}

word computer_get_mem(struct computer* comp, int addr) {
	while (addr >= comp->mem_sz) {
		computer_double_memsz(comp);
		printf("INFO: setting memory size to %d due to read from addres %d\n", comp->mem_sz, addr);
	}
	return comp->mem[addr];
}

word computer_get_parm(struct computer* comp, int pc_offset, int mode) {
	word operand = computer_get_mem(comp, comp->pc + pc_offset);
	operand += mode == 2 ? comp->base : 0;
	return mode == 1 ? operand : computer_get_mem(comp, operand);
}

void read_prog_from_file(struct computer* comp, char* fname) {
	FILE* fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "Error opening file %s\n", fname);
		return;
	}
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, fp);

	char* l = line;
	word x;
	comp->prog_sz = 0;
	while (read_int(&l, &x))
		computer_set_mem(comp, comp->prog_sz++, x);

	free(line);
	fclose(fp);
}


////////////////// DISASM //////////////////////////
void computer_print_parm(struct computer* comp, int pc_offset, int mode) {
	word operand = computer_get_mem(comp, comp->pc + pc_offset);
	if (mode == 0)
		printf("[%" PRId64 "]", operand);
	else if (mode == 1)
		printf("%" PRId64, operand);
	else
		printf("[B + %" PRId64 "]", operand);
}

void disasm(struct computer* comp) {
	int pc_sav = comp->pc;
	while (comp->pc < comp->prog_sz) {
		printf("%4d: ", comp->pc);
		word instr = computer_get_mem(comp, comp->pc);
		int m = instr / 100;
		int opcode = instr % 100;
		int mode[3];
		for (int ii = 0; ii < 3; ++ii) {
			mode[ii] = m % 10;
			m /= 10;
		}

		switch (opcode) {
			case ADD:
				computer_print_parm(comp, 3, mode[2]);
				printf(" = ");
				computer_print_parm(comp, 1, mode[0]);
				printf(" + ");
				computer_print_parm(comp, 2, mode[1]);
				printf("\n");
				comp->pc += 4;
				break;
			case MUL:
				computer_print_parm(comp, 3, mode[2]);
				printf(" = ");
				computer_print_parm(comp, 1, mode[0]);
				printf(" * ");
				computer_print_parm(comp, 2, mode[1]);
				printf("\n");
				comp->pc += 4;
				break;
			case INP:
				computer_print_parm(comp, 1, mode[0]);
				printf(" <- in\n");
				comp->pc += 2;
				break;
			case OUT:
				computer_print_parm(comp, 1, mode[0]);
				printf(" -> out\n");
				comp->pc += 2;
				break;
			case JNE:
				printf("if (");
				computer_print_parm(comp, 1, mode[0]);
				printf(" != 0) goto ");
				computer_print_parm(comp, 2, mode[1]);
				printf("\n");
				comp->pc += 3;
				break;
			case JEQ:
				printf("if (");
				computer_print_parm(comp, 1, mode[0]);
				printf(" == 0) goto ");
				computer_print_parm(comp, 2, mode[1]);
				printf("\n");
				comp->pc += 3;
				break;
			case LT:
				computer_print_parm(comp, 3, mode[2]);
				printf(" = (");
				computer_print_parm(comp, 1, mode[0]);
				printf(" < ");
				computer_print_parm(comp, 2, mode[1]);
				printf(") ? 1 : 0\n");
				comp->pc += 4;
				break;
			case EQ:
				computer_print_parm(comp, 3, mode[2]);
				printf(" = (");
				computer_print_parm(comp, 1, mode[0]);
				printf(" == ");
				computer_print_parm(comp, 2, mode[1]);
				printf(") ? 1 : 0\n");
				comp->pc += 4;
				break;
			case INCB:
				printf("B += ");
				computer_print_parm(comp, 1, mode[0]);
				printf("\n");
				comp->pc += 2;
				break;
			case HLT:
				printf("HALT\n");
				comp->pc += 1;
				break;
			default:
				printf(" -- #%" PRId64 "\n", computer_get_mem(comp, comp->pc));
				comp->pc += 1;
		}
	}
	comp->pc = pc_sav;
}
///////////////// END DISASM //////////////////////////////

void run_program(struct computer* comp) {
	while (computer_get_mem(comp, comp->pc) != HLT) {
		word instr = computer_get_mem(comp, comp->pc);
		int m = instr / 100;
		int opcode = instr % 100;
		int mode[3];
		for (int ii = 0; ii < 3; ++ii) {
			mode[ii] = m % 10;
			m /= 10;
		}

		int x;
		int addr;
		switch (opcode) {
			case ADD:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) + computer_get_parm(comp, 2, mode[1]));
				comp->pc += 4;
				break;
			case MUL:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) * computer_get_parm(comp, 2, mode[1]));
				comp->pc += 4;
				break;
			case INP:
				printf("Value: ");
				scanf("%d", &x);
				addr = computer_get_mem(comp, comp->pc + 1) + (mode[0] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, x);
				comp->pc += 2;
				break;
			case OUT:
				printf("(OUT @ pc=%d) %" PRId64 "\n", comp->pc, computer_get_parm(comp, 1, mode[0]));
				comp->pc += 2;
				break;
			case JNE:
				if (computer_get_parm(comp, 1, mode[0]) != 0)
					comp->pc = computer_get_parm(comp, 2, mode[1]);
				else
					comp->pc += 3;
				break;
			case JEQ:
				if (computer_get_parm(comp, 1, mode[0]) == 0)
					comp->pc = computer_get_parm(comp, 2, mode[1]);
				else
					comp->pc += 3;
				break;
			case LT:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) < computer_get_parm(comp, 2, mode[1]) ? 1 : 0);
				comp->pc += 4;
				break;
			case EQ:
				addr = computer_get_mem(comp, comp->pc + 3) + (mode[2] == 2 ? comp->base : 0);
				computer_set_mem(comp, addr, computer_get_parm(comp, 1, mode[0]) == computer_get_parm(comp, 2, mode[1]) ? 1 : 0);
				comp->pc += 4;
				break;
			case INCB:
				comp->base += computer_get_parm(comp, 1, mode[0]);
				comp->pc += 2;
				break;
			default:
				fprintf(stderr, "Invalid opcode %d @ %d\n", opcode, comp->pc);
				exit(1);
		}
	}
}

void computer_init(struct computer* comp, char* progfname) {
	comp->mem_sz = INITIAL_MEMSZ;
	comp->mem = calloc(comp->mem_sz, sizeof(word));
	read_prog_from_file(comp, progfname);
	comp->pc = 0;
	comp->base = 0;
}

void destroy_computer(struct computer* comp) {
	free(comp->mem);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s progfile\n", argv[0]);
		printf("     : %s -d progfile    for disassembly\n", argv[0]);
		return 1;
	}
	int prg_idx = 1;
	bool do_disassembly = false;
	if (argc >= 3 && !strncmp(argv[1], "-d", 2)) {
		++prg_idx;
		do_disassembly = true;
	}

	struct computer comp;
	computer_init(&comp, argv[prg_idx]);
	
	if (do_disassembly)
		disasm(&comp);
	else {
		run_program(&comp);
	}

	destroy_computer(&comp);
	return 0;
}
