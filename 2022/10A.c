#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define MAXPROGLEN 512

enum opcode {
	ADDX,
	NOOP,

	NR_OPCODES
};

struct instruction {
	enum opcode opcode;
	int         param1;
};

struct program {
	int                proglen; // nr of instructions
	struct instruction instr[MAXPROGLEN];
};

struct instr_info {
	char* mnemonic;
	int   nr_params;
	int   cycles;
};

static const
struct instr_info instr_info[] = {
	{ "addx", 1, 2 },
	{ "noop", 0, 1 }
};

struct computer {
	int            x;
	int            pc;
	int            time; // in cycles
	struct program program;
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	if (isdigit(*line) && line > *pLine && *(line - 1) == '-')
		isneg = true;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return isneg ? (-num) : num;
}

bool read_opcode(char** pLine, enum opcode* opcode_out) {
	char* line = *pLine;
	while (*line && isspace(*line))
		++line;
	enum opcode opcode;
	for (opcode = 0; opcode < NR_OPCODES; ++opcode) {
		char* mnemonic = instr_info[opcode].mnemonic;
		int len = strlen(mnemonic);
		if (strncmp(line, mnemonic, len) == 0) {
			line += len;
			break;
		}
	}
	if (opcode < NR_OPCODES)
		*opcode_out = opcode;
	*pLine = line;
	return (opcode < NR_OPCODES);
}

void print_program(struct program* program) {
	for (int ii = 0; ii < program->proglen; ++ii) {
		struct instruction* instr = program->instr + ii;
		enum opcode opcode = instr->opcode;
		int param = instr->param1;
		if (instr_info[opcode].nr_params)
			printf("%s %d\n", instr_info[opcode].mnemonic, param);
		else
			printf("%s\n", instr_info[opcode].mnemonic);
	}
}

bool do_single_instruction(struct computer* comp) {
	if (comp->pc >= comp->program.proglen)
		return false;
	struct instruction* instr = &comp->program.instr[comp->pc];
	++comp->pc;
	comp->time += instr_info[instr->opcode].cycles;
	switch (instr->opcode) {
		case ADDX:
			comp->x += instr->param1;
			break;
		case NOOP:
			break;
		default:
			printf("Unknown opcode %d\n", instr->opcode);
			return false;
	}
	return true;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	struct computer computer;
	computer.program.proglen = 0;
	computer.pc   = 0;
	computer.x    = 1;
	computer.time = 0;

	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		enum opcode opcode = NR_OPCODES;
		if (read_opcode(&l, &opcode)) {
			int param = 0;
			if (instr_info[opcode].nr_params)
				param = read_int(&l);
			computer.program.instr[computer.program.proglen].opcode = opcode;
			computer.program.instr[computer.program.proglen].param1 = param;
			++computer.program.proglen;
		}
	}
	free(line);
	// execute, keep track of "sig strengths"

	int timestamps[] = { 20, 60, 100, 140, 180, 220 };
	int nr_timestamps = sizeof(timestamps) / sizeof(timestamps[0]);
	int timestamp_idx = 0;

	int sum = 0;
	while (timestamp_idx < nr_timestamps) {
		int time_before = computer.time;
		int x_before = computer.x;
		do_single_instruction(&computer);
		int time_after = computer.time;
		int ts = timestamps[timestamp_idx];
		if (time_before < ts && time_after >= ts) {
			sum += ts * x_before;
			++timestamp_idx;
		}
	}
	printf("%d\n", sum);
	return 0;
}
