#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define NR_REGS    4

enum mnemonic {
	ADDR = 0, ADDI,
	MULR, MULI,
	BANR, BANI,
	BORR, BORI,
	SETR, SETI,
	GTIR, GTRI, GTRR,
	EQIR, EQRI, EQRR,

	NR_OPCODES
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
		default:
			break;
	}
}

struct prog_line {
	int opcode;
	int operands[3];
	struct prog_line* next; // LL
};

void check_mnemonics(struct instr* instr, bool* possible_map) {
	// can opcode with nr x represent mnemonic m? idx = x * NR_OPCODES + m
	int regs[NR_REGS];
	int x = instr->opcode;
	for (enum mnemonic m = ADDR; m <= EQRR; ++m) {
		for (int ii = 0; ii < NR_REGS; ++ii)
			regs[ii] = instr->before[ii];
		do_mnem(m, instr->operands, regs);
		bool same = true;
		for (int ii = 0; ii < NR_REGS; ++ii)
			same = same && regs[ii] == instr->after[ii];
		if (!same)
			possible_map[x * NR_OPCODES + m] = false;
	}
}

bool solve_map(bool* map) {
	// solve map: each row and each col can only contain 1 true

	bool changed = true;
	while (changed) {
		// find rows that have single true already: all other rows have false there
		changed = false;
		for (int r = 0; r < NR_OPCODES; ++r) {
			int count = 0;
			int last_true_col = 0;
			for (int c = 0; c < NR_OPCODES; ++c) {
				bool v = map[r * NR_OPCODES + c];
				count += v ? 1 : 0;
				last_true_col = v ? c : last_true_col;
			}
			if (count == 1) {
				for (int rr = 0; rr < NR_OPCODES; ++rr) {
					int idx = rr * NR_OPCODES + last_true_col;
					if (rr != r && map[idx]) {
						map[idx] = false;
						changed= true;
					}
				}
			}
		}
		// find cols that have single true already: all other cols have false there
		for (int c = 0; c < NR_OPCODES; ++c) {
			int count = 0;
			int last_true_row = 0;
			for (int r = 0; r < NR_OPCODES; ++r) {
				bool v = map[r * NR_OPCODES + c];
				count += v ? 1 : 0;
				last_true_row = v ? r : last_true_row;
			}
			if (count == 1) {
				for (int cc = 0; cc < NR_OPCODES; ++cc) {
					int idx = last_true_row * NR_OPCODES + cc;
					if (cc != c && map[idx]) {
						map[idx] = false;
						changed= true;
					}
				}
			}
		}
	}
	// check solution
	for (int r = 0; r < NR_OPCODES; ++r) {
		int count = 0;
		for (int c = 0; c < NR_OPCODES; ++c)
			count += map[r * NR_OPCODES + c] ? 1 : 0;
		if (count != 1)
			return false;
	}
	return true;
}



int main(int argc, char* argv[]) {
	bool possible_map[NR_OPCODES * NR_OPCODES]; // can opcode with nr x represent mnemonic m? idx = x * NR_OPCODES + m
	for (int ii = 0; ii < NR_OPCODES * NR_OPCODES; ++ii)
		possible_map[ii] = true;

	struct prog_line* prog = NULL;
	struct prog_line* prog_tail = NULL;

	char *line = NULL;
	size_t len = 0;
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
			check_mnemonics(&instr, possible_map);
		}
		else {
			char* l = line;
			struct prog_line* prgline = malloc(sizeof(struct prog_line));
			prgline->next = NULL;
			prgline->opcode = read_int(&l);
			for (int ii = 0; ii < 3; ++ii)
				prgline->operands[ii] = read_int(&l);
			if (!prog)
				prog = prgline;
			else
				prog_tail->next = prgline;
			prog_tail = prgline;
		}
	}
	free(line);

	// solve map: each row and each col can only contain 1 true
	if (!solve_map(possible_map))
		printf("Could not solve map\n");
	else {
		// create simpler 1-to-1 map
		int opcode_map[NR_OPCODES];
		for (int opcode = 0; opcode < NR_OPCODES; ++opcode) {
			for (int m = 0; m < NR_OPCODES; ++m) {
				if (possible_map[opcode * NR_OPCODES + m]) {
					opcode_map[opcode] = m;
					break;
				}
			}
		}
		// run program
		int regs[NR_REGS] = {0};
		for (struct prog_line* pl = prog; pl; pl = pl->next)
			do_mnem(opcode_map[pl->opcode], pl->operands, regs);
		printf("%d\n", regs[0]);
	}

	while (prog) {
		struct prog_line* pl = prog;
		prog = prog->next;
		free(pl);
	}
	return 0;
}
