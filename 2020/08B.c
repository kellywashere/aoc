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

bool run_computer(struct computer* computer) { // returns true when no loop detected
	computer->pc = 0;
	computer->acc = 0;
	for (int ii = 0; ii < MAXPROGLEN; ++ii)
		computer->instr_run_before[ii] = false;

	while (computer->pc >= 0 && computer->pc < computer->program->proglen) {
		// inf loop detection
		if (computer->instr_run_before[computer->pc])
			return false;
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
	return (computer->pc == computer->program->proglen); // normal ending
}

bool toggle_instr(struct program* p, int idx) {
	struct instruction* instr = p->instr + idx;
	if (instr->opcode == NOP) {
		instr->opcode = JMP;
		return true;
	}
	else if (instr->opcode == JMP) {
		instr->opcode = NOP;
		return true;
	}
	return false;
}

int main(int argc, char* argv[]) {
	struct program program;
	struct computer computer;

	read_program_from_stdin(&program);
	computer.program = &program;

	int64_t acc_found = 0;
	for (int ii = 0; !acc_found && ii < program.proglen; ++ii) {
		if (toggle_instr(&program, ii)) {
			if (run_computer(&computer))
				acc_found = computer.acc;
			toggle_instr(&program, ii);
		}
	}

	printf("%" PRIi64 "\n", acc_found);

	return 0;
}
