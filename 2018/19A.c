#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NR_REGS    6
#define PROG_MEM 512

enum mnemonic {
	ADDR, ADDI,
	MULR, MULI,
	BANR, BANI,
	BORR, BORI,
	SETR, SETI,
	GTIR, GTRI, GTRR,
	EQIR, EQRI, EQRR,

	NR_MNEM
};

char* mnem_str[] = {
	"addr", "addi",
	"mulr", "muli",
	"banr", "bani",
	"borr", "bori",
	"setr", "seti",
	"gtir", "gtri", "gtrr",
	"eqir", "eqri", "eqrr"
};

struct instr {
	enum mnemonic opcode;
	int           operands[3];
};

struct computer {
	int          ip;
	int          regs[NR_REGS];
	bool         ip_linked;
	int          ip_reg;
	struct instr program[PROG_MEM];
	int          proglen;
};

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

void reset_computer(struct computer* c) {
	c->ip = 0;
	for (int ii = 0; ii < NR_REGS; ++ii)
		c->regs[ii] = 0;
	c->ip_linked = false;
	c->proglen = 0;
}

void print_regs(struct computer* c) {
	printf("[%d", c->regs[0]);
	for (int ii = 1; ii < NR_REGS; ++ii)
		printf(", %d", c->regs[ii]);
	printf("]");
}

void print_instr(struct instr* i) {
	printf("%s %d %d %d", mnem_str[i->opcode], i->operands[0], i->operands[1], i->operands[2]);
}

void run_instr(struct computer* c, bool debug) {
	struct instr* instr = &c->program[c->ip];

	// ip=0 [0, 0, 0, 0, 0, 0] seti 5 0 1 [0, 5, 0, 0, 0, 0]
	if (debug) {
		printf("ip=%d ", c->ip);
		print_regs(c);
		printf(" ");
		print_instr(instr);
		printf(" ");
	}

	int targetreg = instr->operands[2];
	if (c->ip_linked)
		c->regs[c->ip_reg] = c->ip;
	switch (instr->opcode) {
		case ADDR:
			c->regs[targetreg] = c->regs[instr->operands[0]] + c->regs[instr->operands[1]];
			break;
		case ADDI:
			c->regs[targetreg] = c->regs[instr->operands[0]] + instr->operands[1];
			break;
		case MULR:
			c->regs[targetreg] = c->regs[instr->operands[0]] * c->regs[instr->operands[1]];
			break;
		case MULI:
			c->regs[targetreg] = c->regs[instr->operands[0]] * instr->operands[1];
			break;
		case BANR:
			c->regs[targetreg] = c->regs[instr->operands[0]] & c->regs[instr->operands[1]];
			break;
		case BANI:
			c->regs[targetreg] = c->regs[instr->operands[0]] & instr->operands[1];
			break;
		case BORR:
			c->regs[targetreg] = c->regs[instr->operands[0]] | c->regs[instr->operands[1]];
			break;
		case BORI:
			c->regs[targetreg] = c->regs[instr->operands[0]] | instr->operands[1];
			break;
		case SETR:
			c->regs[targetreg] = c->regs[instr->operands[0]];
			break;
		case SETI:
			c->regs[targetreg] = instr->operands[0];
			break;
		case GTIR:
			c->regs[targetreg] = instr->operands[0] > c->regs[instr->operands[1]] ? 1 : 0;
			break;
		case GTRI:
			c->regs[targetreg] = c->regs[instr->operands[0]] > instr->operands[1] ? 1 : 0;
			break;
		case GTRR:
			c->regs[targetreg] = c->regs[instr->operands[0]] > c->regs[instr->operands[1]] ? 1 : 0;
			break;
		case EQIR:
			c->regs[targetreg] = instr->operands[0] == c->regs[instr->operands[1]] ? 1 : 0;
			break;
		case EQRI:
			c->regs[targetreg] = c->regs[instr->operands[0]] == instr->operands[1] ? 1 : 0;
			break;
		case EQRR:
			c->regs[targetreg] = c->regs[instr->operands[0]] == c->regs[instr->operands[1]] ? 1 : 0;
			break;
		default:
			break;
	}
	if (c->ip_linked)
		c->ip = c->regs[c->ip_reg];
	++c->ip;
	if (debug) {
		print_regs(c);
		printf(" ip=%d\n", c->ip);
	}
}

void run_program(struct computer* c, bool debug) {
	c->ip = 0;
	while (c->ip < c->proglen)
		run_instr(c, debug);
}

int main(int argc, char* argv[]) {
	struct computer computer;
	reset_computer(&computer);

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		for (int m = 0; m < NR_MNEM; ++m) {
			char* l = line;
			if (!strncmp(line, "#ip", 3)) {
				computer.ip_reg = read_int(&l);
				computer.ip_linked = true;
			}
			else if (!strncmp(line, mnem_str[m], strlen(mnem_str[m]))) {
				computer.program[computer.proglen].opcode = m;
				for (int ii = 0; ii < 3; ++ii)
					computer.program[computer.proglen].operands[ii] = read_int(&l);
				++computer.proglen;
				break;
			}
		}
	}

	run_program(&computer, false);
	// print_regs(&computer); printf("\n");
	printf("%d\n", computer.regs[0]);

	free(line);
	return 0;
}
