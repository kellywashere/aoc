#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>

#define MAXPROGLEN 1024
#define NR_REGS    16

char* next_word(char* line) {
	while (*line && !isspace(*line))
		++line;
	while (*line && isspace(*line))
		++line;
	return line;
}

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? -num : num;
}

int read_reg(char** pLine) {
	char* line = *pLine;
	while (*line && (*line < 'a' || *line > ('a' + NR_REGS - 1)))
		++line;
	int regnr = *line - 'a';
	*pLine = line + 1;
	return regnr;
}

int read_reg_or_int(char** pLine, bool* is_int) {
	char* l = *pLine;
	int x;
	while (*l && !(*l == '-' || (*l >= '0' && *l <= '9') || (*l >= 'a' && *l <= ('a' + NR_REGS - 1))))
		++l;
	if (*l >= 'a' && *l <= ('a' + NR_REGS - 1)) {
		*is_int = false;
		x = read_reg(&l);
	}
	else {
		*is_int = true;
		x = read_int(&l);
	}
	*pLine = l;
	return x;
}

enum opcode {
	SET,
	SUB,
	MUL,
	JNZ
};

struct instruction {
	enum opcode opcode;
	bool        immediate1;
	int         op1;
	bool        immediate2;
	int         op2;
};

struct program {
	int                proglen; // nr of instructions
	struct instruction instr[MAXPROGLEN];
};

struct computer {
	int64_t reg[NR_REGS];
	int     pc;
	struct  program* program;
	int     mul_count;
};

void read_program_from_stdin(struct program* program) {
	int idx = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		while (*l && isspace(*l))
			++l;
		if (!strncmp(l, "set", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = SET;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "sub", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = SUB;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "mul", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = MUL;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "jnz", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = JNZ;
			program->instr[idx].op1 = read_reg_or_int(&l, &program->instr[idx].immediate1);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
	}
	program->proglen = idx;
	free(line);
}

void run_computer(struct computer* computer) {
	int64_t offs;
	int64_t x;
	computer->pc = 0;
	while (computer->pc < computer->program->proglen) {
		struct instruction* instr = computer->program->instr + computer->pc++;
		switch (instr->opcode) {
			case SET:
				computer->reg[instr->op1] = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				break;
			case SUB:
				x = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				computer->reg[instr->op1] -= x;
				break;
			case MUL:
				x = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				computer->reg[instr->op1] *= x;
				++computer->mul_count;
				break;
			case JNZ:
				x = instr->immediate1 ? instr->op1 : computer->reg[instr->op1];
				offs = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				if (x != 0)
					computer->pc += offs - 1; // -1: already inc-ed by 1 before
				break;
		}
	}
}

int main(int argc, char* argv[]) {
	struct program program;
	struct computer computer;

	read_program_from_stdin(&program);

	for (int ii = 0; ii < NR_REGS; ++ii)
		computer.reg[ii] = 0;

	for (int ii = 1; ii < argc; ++ii)
		computer.reg[ii - 1] = atoi(argv[ii]);

	computer.pc = 0;
	computer.program = &program;
	computer.mul_count = 0;

	run_computer(&computer);

	printf("%d\n", computer.mul_count);

	return 0;
}
