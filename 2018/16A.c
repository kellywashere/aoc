#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define NR_REGS 4

enum mnemonic {
	ADDR, ADDI,
	MULR, MULI,
	BANR, BANI,
	BORR, BORI,
	SETR, SETI,
	GTIR, GTRI, GTRR,
	EQIR, EQRI, EQRR
};

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

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

struct instr {
	int before[NR_REGS];
	int after[NR_REGS];
	int opcode;
	int operands[3];
};

void do_mnem(enum mnemonic mnem, int* operands, int* regs) {
	switch (mnem) {
		case ADDR:
			regs[operands[2]] = regs[operands[0]] + regs[operands[1]];
			break;
		case ADDI:
			regs[operands[2]] = regs[operands[0]] + operands[1];
			break;
		case MULR:
			regs[operands[2]] = regs[operands[0]] * regs[operands[1]];
			break;
		case MULI:
			regs[operands[2]] = regs[operands[0]] * operands[1];
			break;
		case BANR:
			regs[operands[2]] = regs[operands[0]] & regs[operands[1]];
			break;
		case BANI:
			regs[operands[2]] = regs[operands[0]] & operands[1];
			break;
		case BORR:
			regs[operands[2]] = regs[operands[0]] | regs[operands[1]];
			break;
		case BORI:
			regs[operands[2]] = regs[operands[0]] | operands[1];
			break;
		case SETR:
			regs[operands[2]] = regs[operands[0]];
			break;
		case SETI:
			regs[operands[2]] = operands[0];
			break;
		case GTIR:
			regs[operands[2]] = operands[0] > regs[operands[1]] ? 1 : 0;
			break;
		case GTRI:
			regs[operands[2]] = regs[operands[0]] > operands[1] ? 1 : 0;
			break;
		case GTRR:
			regs[operands[2]] = regs[operands[0]] > regs[operands[1]] ? 1 : 0;
			break;
		case EQIR:
			regs[operands[2]] = operands[0] == regs[operands[1]] ? 1 : 0;
			break;
		case EQRI:
			regs[operands[2]] = regs[operands[0]] == operands[1] ? 1 : 0;
			break;
		case EQRR:
			regs[operands[2]] = regs[operands[0]] == regs[operands[1]] ? 1 : 0;
			break;
	}
}

int count_mnemonics(struct instr* instr) {
	int regs[NR_REGS];
	int count = 0;
	for (enum mnemonic m = ADDR; m <= EQRR; ++m) {
		for (int ii = 0; ii < NR_REGS; ++ii)
			regs[ii] = instr->before[ii];
		do_mnem(m, instr->operands, regs);
		bool same = true;
		for (int ii = 0; ii < NR_REGS; ++ii)
			same = same && regs[ii] == instr->after[ii];
		count += same ? 1 : 0;
	}
	return count;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int count = 0;
	while (getline(&line, &len, stdin) != -1) {
		if (empty_line(line))
			continue;
		if (line[0] == 'B') { // before
			char* l = line;
			struct instr instr;
			for (int ii = 0; ii < NR_REGS; ++ii)
				instr.before[ii] = read_int(&l);
			getline(&line, &len, stdin);
			l = line;
			instr.opcode = read_int(&l);
			for (int ii = 0; ii < 3; ++ii)
				instr.operands[ii] = read_int(&l);
			getline(&line, &len, stdin);
			l = line;
			for (int ii = 0; ii < NR_REGS; ++ii)
				instr.after[ii] = read_int(&l);
			count += count_mnemonics(&instr) >= 3 ? 1 : 0;
		}
		else
			break; // we're done, second part of input skipped
	}
	printf("%d\n", count);
	free(line);
	return 0;
}
