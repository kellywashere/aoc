#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <string.h>

// base: ../2017/18A.c

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

enum opcode {
	ACC,
	JMP,
	NOP
};

struct instruction {
	enum opcode opcode;
	int         op;
};

struct program {
	int                proglen; // nr of instructions
	struct instruction instr[MAXPROGLEN];
};

struct computer {
	int64_t acc;
	int     pc;
	struct  program* program;

	bool    instr_run_before[MAXPROGLEN]; // problem A
};

void read_program_from_stdin(struct program* program) {
	int idx = 0;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		while (*l && isspace(*l))
			++l;
		if (!strncmp(l, "acc", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = ACC;
			program->instr[idx].op = read_int(&l);
			++idx;
		}
		else if (!strncmp(l, "jmp", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = JMP;
			program->instr[idx].op = read_int(&l);
			++idx;
		}
		else if (!strncmp(l, "nop", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = NOP;
			program->instr[idx].op = read_int(&l);
			++idx;
		}
	}
	program->proglen = idx;
	free(line);
}

void run_computer(struct computer* computer) {
	computer->pc = 0;
	computer->acc = 0;
	while (computer->pc >= 0 && computer->pc < computer->program->proglen) {
		if (computer->instr_run_before[computer->pc])
			break; // problem 08A
		computer->instr_run_before[computer->pc] = true;

		struct instruction* instr = computer->program->instr + computer->pc++;
		switch (instr->opcode) {
			case ACC:
				computer->acc += instr->op;
				break;
			case JMP:
				computer->pc += instr->op - 1; // -1: already inc-ed by 1 before
				break;
			default:
				;
		}
	}
}

int main(int argc, char* argv[]) {
	struct program program;
	struct computer computer;

	read_program_from_stdin(&program);
	computer.program = &program;

	for (int ii = 0; ii < MAXPROGLEN; ++ii)
		computer.instr_run_before[ii] = false;

	run_computer(&computer);

	printf("%" PRIi64 "\n", computer.acc);

	return 0;
}
