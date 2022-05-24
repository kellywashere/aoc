#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>

#define MAXPROGLEN 1024

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
	while (*line && (*line < 'a' || *line > 'b'))
		++line;
	int regnr = *line - 'a';
	*pLine = line + 1;
	return regnr;
}

enum opcode {
	HLF,
	TPL,
	INC,
	JMP,
	JIE,
	JIO
};

struct instruction {
	enum opcode opcode;
	int         op1;
	int         op2;
};

struct program {
	int                proglen; // nr of instructions
	struct instruction instr[MAXPROGLEN];
};

struct computer {
	int64_t reg[2];
	int     pc;
	struct  program* program;
};

void read_program_from_stdin(struct program* program) {
	int idx = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		while (*l && isspace(*l))
			++l;
		if (!strncmp(l, "hlf", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = HLF;
			program->instr[idx].op1 = read_reg(&l);
			++idx;
		}
		else if (!strncmp(l, "tpl", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = TPL;
			program->instr[idx].op1 = read_reg(&l);
			++idx;
		}
		else if (!strncmp(l, "inc", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = INC;
			program->instr[idx].op1 = read_reg(&l);
			++idx;
		}
		else if (!strncmp(l, "jmp", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = JMP;
			program->instr[idx].op1 = read_int(&l);
			++idx;
		}
		else if (!strncmp(l, "jie", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = JIE;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_int(&l);
			++idx;
		}
		else if (!strncmp(l, "jio", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = JIO;
			program->instr[idx].op1 = read_reg(&l);
			program->instr[idx].op2 = read_int(&l);
			++idx;
		}
	}
	program->proglen = idx;
	free(line);
}

void run_computer(struct computer* computer) {
	computer->pc = 0;
	while (computer->pc < computer->program->proglen) {
		struct instruction* instr = computer->program->instr + computer->pc++;
		switch (instr->opcode) {
			case HLF:
				computer->reg[instr->op1] /= 2;
				break;
			case TPL:
				computer->reg[instr->op1] *= 3;
				break;
			case INC:
				++computer->reg[instr->op1];
				break;
			case JMP:
				computer->pc += instr->op1 - 1; // -1: already inc-ed by 1 before
				break;
			case JIE:
				if (computer->reg[instr->op1] % 2 == 0)
					computer->pc += instr->op2 - 1; // -1: already inc-ed by 1 before
				break;
			case JIO:
				if (computer->reg[instr->op1] == 1)
					computer->pc += instr->op2 - 1; // -1: already inc-ed by 1 before
				break;
		}
	}
}

int main(int argc, char* argv[]) {
	struct program program;
	struct computer computer;

	read_program_from_stdin(&program);

	for (int ii = 0; ii < 2; ++ii)
		computer.reg[ii] = 0;

	computer.pc = 0;
	computer.program = &program;

	run_computer(&computer);
	printf("%" PRId64 "\n", computer.reg[1]);

	return 0;
}
