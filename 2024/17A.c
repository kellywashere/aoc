#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>

#include <assert.h>

#define MAX_PROGSIZE 128

enum mnemonic {
	ADV = 0,
	BXL = 1,
	BST = 2,
	JNZ = 3,
	BXC = 4,
	OUT = 5,
	BDV = 6,
	CDV = 7
};

// For disassembly:
char* mnemstr[] = {
	"ADV",
	"BXL",
	"BST",
	"JNZ",
	"BXC",
	"OUT",
	"BDV",
	"CDV"
};

// For disassembly:
enum operand_type {
	LITERAL,
	COMBO,
	IGNORE
};
enum operand_type optype[] = {
	COMBO,
	LITERAL,
	COMBO,
	LITERAL,
	IGNORE,
	COMBO,
	COMBO,
	COMBO
};

typedef unsigned char byte;
typedef unsigned int  uint;

struct computer {
	uint64_t  regs[3];

	uint      pc;
	bool      halted;
	bool      write_comma; // for comma ctrl OUT instr

	size_t    prog_sz;
	byte      prog[MAX_PROGSIZE];
};

bool read_uint64(const char** pLine, uint64_t* x) {
	uint64_t num = 0;
	const char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

struct computer* computer_create(void) {
	struct computer* c = calloc(1, sizeof(struct computer));
	c->pc = 0;
	c->prog_sz = 0;
	c->halted = false;
	c->write_comma = false;
	return c;
}

void computer_destroy(struct computer* c) {
	if (c) {
		free(c);
	}
}

void computer_set_reg(struct computer* c, uint idx, uint64_t val) {
	assert(idx < 3);
	c->regs[idx] = val;
}

void computer_add_instr(struct computer* c, uint opcode, uint operand) {
	assert(c->prog_sz + 2 <= MAX_PROGSIZE);
	c->prog[c->prog_sz++] = opcode;
	c->prog[c->prog_sz++] = operand;
}

uint64_t computer_get_operand_val(struct computer* c, uint opr) {
	assert(opr < 7);
	if (opr <= 3)
		return opr;
	return c->regs[opr - 4];
}

// for disassembly:
void combo_to_string(uint opr, char* buf) {
	assert(opr < 7);
	if (opr <= 3)
		sprintf(buf, "%c", '0' + opr);
	else
		sprintf(buf, "reg%c", 'A' + opr - 4);
}

bool computer_run_instr(struct computer* c) {
	// returns false when halted
	if (c->halted)
		return false;
	if (c->pc >= c->prog_sz) {
		c->halted = true;
		return false;
	}

	uint opcode  = c->prog[c->pc++];
	uint operand = c->prog[c->pc++];

	uint64_t outval;
	switch (opcode) {
		case ADV:
			c->regs[0] >>= computer_get_operand_val(c, operand);
			break;
		case BXL:
			c->regs[1] = c->regs[1] ^ operand;
			break;
		case BST:
			c->regs[1] = computer_get_operand_val(c, operand) & 0x7;
			break;
		case JNZ:
			if (c->regs[0] != 0) {
				c->pc = operand;
			}
			break;
		case BXC:
			c->regs[1] = c->regs[1] ^ c->regs[2];
			break;
		case OUT:
			if (c->write_comma)
				printf(",");
			outval = computer_get_operand_val(c, operand) & 0x7;
			printf("%" PRIu64, outval);
			c->write_comma = true;
			break;
		case BDV:
			c->regs[1] = c->regs[0] >> computer_get_operand_val(c, operand);
			break;
		case CDV:
			c->regs[2] = c->regs[0] >> computer_get_operand_val(c, operand);
			break;
	}

	return true;
}

void computer_disassemble(struct computer* c) {
	for (uint addr = 0; addr < c->prog_sz; addr += 2) {
		uint opcode  = c->prog[addr];
		int operand = c->prog[addr + 1];
		char str[16];
		printf("%04d: %s", addr, mnemstr[opcode]);
		switch (optype[opcode]) {
			case LITERAL:
				printf(" %u\n", operand);
				break;
			case COMBO:
				combo_to_string(operand, str);
				printf(" %s\n", str);
				break;
			case IGNORE:
				printf("\n");
				break;
		}
	}
}

void computer_disassemble_readable(struct computer* c) {
	for (uint addr = 0; addr < c->prog_sz; addr += 2) {
		uint opcode  = c->prog[addr];
		int operand = c->prog[addr + 1];
		char str[16];
		printf("%04d: ", addr);
		switch (opcode) {
			case ADV:
				combo_to_string(operand, str);
				printf("regA = regA >> %s\n", str);
				break;
			case BXL:
				printf("regB = regB ^ %u\n", operand);
				break;
			case BST:
				combo_to_string(operand, str);
				printf("regB = %s & 0x7\n", str);
				break;
			case JNZ:
				printf("if (regA != 0) goto %u\n", operand);
				break;
			case BXC:
				printf("regB = regB ^ regC\n");
				break;
			case OUT:
				combo_to_string(operand, str);
				printf("output (%s & 0x7)\n", str);
				break;
			case BDV:
				combo_to_string(operand, str);
				printf("regB = regA >> %s\n", str);
				break;
			case CDV:
				combo_to_string(operand, str);
				printf("regC = regA >> %s\n", str);
				break;
		}
	}
}


void computer_run(struct computer* c) {
	while (computer_run_instr(c))
		;
}

int main(int argc, char* argv[]) {
	int vals_read = 0;
	uint64_t x; // for reading in vals

	struct computer* computer = computer_create();

	char *line = NULL;
	size_t len = 0;

	// read init vals of regs
	while (getline(&line, &len, stdin) != -1 && vals_read < 3) {
		const char* l = line;
		read_uint64(&l, &x);
		computer_set_reg(computer, vals_read, x);
		++vals_read;
	}
	// Read program
	vals_read = 0;
	uint vals[2];
	while (getline(&line, &len, stdin) != -1) {
		const char* l = line;
		while (read_uint64(&l, &x)) {
			vals[vals_read] = (uint)x;
			++vals_read;
			if (vals_read == 2) {
				computer_add_instr(computer, vals[0], vals[1]);
				vals_read = 0;
			}
		}
	}

	computer_disassemble_readable(computer);
	printf("\n");
	computer_run(computer);

	computer_destroy(computer);
	free(line);
	return 0;
}
