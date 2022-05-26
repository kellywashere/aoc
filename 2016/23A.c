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
	while (*line && (*line < 'a' || *line > 'd'))
		++line;
	int regnr = *line - 'a';
	*pLine = line + 1;
	return regnr;
}

int read_reg_or_int(char** pLine, bool* is_int) {
	char* l = *pLine;
	int x;
	while (*l && !(*l == '-' || (*l >= '0' && *l <= '9') || (*l >= 'a' && *l <= 'd')))
		++l;
	if (*l >= 'a' && *l <= 'd') {
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
	CPY,
	INC,
	DEC,
	JNZ,
	TGL
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
	int64_t reg[4];
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
		if (!strncmp(l, "cpy", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = CPY;
			program->instr[idx].op1 = read_reg_or_int(&l, &program->instr[idx].immediate1);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "inc", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = INC;
			program->instr[idx].op1 = read_reg(&l);
			++idx;
		}
		else if (!strncmp(l, "dec", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = DEC;
			program->instr[idx].op1 = read_reg(&l);
			++idx;
		}
		else if (!strncmp(l, "jnz", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = JNZ;
			program->instr[idx].op1 = read_reg_or_int(&l, &program->instr[idx].immediate1);
			program->instr[idx].op2 = read_reg_or_int(&l, &program->instr[idx].immediate2);
			++idx;
		}
		else if (!strncmp(l, "tgl", 3)) {
			l = next_word(l);
			program->instr[idx].opcode = TGL;
			program->instr[idx].op1 = read_reg_or_int(&l, &program->instr[idx].immediate1);
			++idx;
		}
	}
	program->proglen = idx;
	free(line);
}

void toggle_instr(struct instruction* instr) {
	switch (instr->opcode) {
		case CPY:
			instr->opcode = JNZ;
			break;
		case INC:
			instr->opcode = DEC;
			break;
		case DEC:
			instr->opcode = INC;
			break;
		case JNZ:
			instr->opcode = CPY;
			break;
		case TGL:
			instr->opcode = JNZ;
			break;
	}
}

void run_computer(struct computer* computer) {
	int offs;
	computer->pc = 0;
	while (computer->pc < computer->program->proglen) {
		struct instruction* instr = computer->program->instr + computer->pc++;
		switch (instr->opcode) {
			case CPY:
				if (!instr->immediate2) // only cpy to reg
					computer->reg[instr->op2] = instr->immediate1 ? instr->op1 : computer->reg[instr->op1];
				break;
			case INC:
				++computer->reg[instr->op1];
				break;
			case DEC:
				--computer->reg[instr->op1];
				break;
			case JNZ:
				bool nonzero = instr->immediate1 ? instr->op1 != 0 : computer->reg[instr->op1] != 0;
				offs = instr->immediate2 ? instr->op2 : computer->reg[instr->op2];
				if (nonzero)
					computer->pc += offs - 1; // -1: already inc-ed by 1 before
				break;
			case TGL:
				offs = instr->immediate1 ? instr->op1 : computer->reg[instr->op1];
				int tgl_addr = computer->pc + offs - 1; // -1: already inc-ed by 1 before
				if (tgl_addr >= 0 && tgl_addr < computer->program->proglen) {
					struct instruction* tglinstr = computer->program->instr + tgl_addr;
					toggle_instr(tglinstr);
				}
				break;
		}
	}
}

int main(int argc, char* argv[]) {
	struct program program;
	struct computer computer;

	read_program_from_stdin(&program);

	for (int ii = 0; ii < 4; ++ii)
		computer.reg[ii] = 0;
	computer.reg[0] = 7;
	computer.pc = 0;
	computer.program = &program;

	run_computer(&computer);
	printf("%" PRId64 "\n", computer.reg[0]);

	return 0;
}
