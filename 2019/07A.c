#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MEMSZ 1024

#define NR_AMPS   5
#define MAX_PHASE 4

enum mnemonics {
	ADD = 1,
	MUL = 2,
	INP = 3,
	OUT = 4,
	JNE = 5,
	JEQ = 6,
	LT  = 7,
	EQ  = 8,

	HLT = 99
};

bool read_int(char** pLine, int* x) {
	int num = 0;
	bool isneg = false;
	char* line = *pLine;
	while (*line && !(*line == '-' || (*line >= '0' && *line <= '9')))
		++line;
	if (*line == '-') {
		isneg = true;
		++line;
	}
	if (*line < '0' || *line > '9')
		return false;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = isneg ? -num : num;
	*pLine = line;
	return true;
}

int read_prog_from_file(int* mem, char* fname) {
	int mem_used = 0;
	FILE* fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "Error opening file %s\n", fname);
		return 0;
	}
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, fp);
	char* l = line;
	int x;
	while (read_int(&l, &x))
		mem[mem_used++] = x;
	free(line);
	fclose(fp);
	return mem_used;
}

int get_parm(int operand, int mode, int* mem) {
	return mode == 0 ? mem[operand] : operand;
}

/*  Read instructions better next time...
bool next_phase(int* phases, int phases_sz, int max) {
	// find first dig from right that can be increased
	int idx = phases_sz - 1;
	while (idx >= 0 && phases[idx] == max)
		--idx;
	if (idx < 0)
		return false;
	++phases[idx++];
	for (; idx < phases_sz; ++idx)
		phases[idx] = 0;
	return true;
}
*/

// From projecteuler, problem 041.c
bool next_perm(int a[], int len) {
	// returns false when no next permutation exists
	// step 1: from right to left, find first element higher than its right neighbor: pivot = a[idxPivot]
	// step 2: sort elements to right of idxPivot (invert order in this case!)
	// step 3: from idxPivot upward, find first element > pivot: idxSwap
	// step 4: swap pivot with a[idxSwap]
	int idxPivot;
	for (idxPivot = len - 2; idxPivot >= 0; --idxPivot) {
		if (a[idxPivot] < a[idxPivot + 1]) {
			// invert order of elements idxPivot+1 ... len-1
			int t;
			int ii = idxPivot + 1;
			int jj = len - 1;
			while (jj > ii) {
				t = a[jj];
				a[jj--] = a[ii];
				a[ii++] = t;
			}
			// from idxPivot upward, find first element > pivot: ii
			for (ii = idxPivot + 1; a[ii] < a[idxPivot]; ++ii);
			// swap a[ii] with a[idxPivot]
			t = a[ii];
			a[ii] = a[idxPivot];
			a[idxPivot] = t;
			break;
		}
	}
	return (idxPivot >= 0);
}


int run_program(int* prog, int prog_sz, int* input) {
	int* mem = malloc(prog_sz * sizeof(int));
	memcpy(mem, prog, prog_sz * sizeof(int));

	int pc = 0;
	int in_idx = 0;
	int outval = 0;
	while (mem[pc] != HLT) {
		int m = mem[pc] / 100;
		int opcode = mem[pc] - 100 * m;
		int mode[3];
		for (int ii = 0; ii < 3; ++ii) {
			mode[ii] = m % 10;
			m /= 10;
		}

		switch (opcode) {
			case ADD:
				mem[mem[pc + 3]] = get_parm(mem[pc + 1], mode[0], mem) + get_parm(mem[pc + 2], mode[1], mem);
				pc += 4;
				break;
			case MUL:
				mem[mem[pc + 3]] = get_parm(mem[pc + 1], mode[0], mem) * get_parm(mem[pc + 2], mode[1], mem);
				pc += 4;
				break;
			case INP:
				if (in_idx > 1)
					fprintf(stderr, "More inputs asked than expected\n");
				else {
					mem[mem[pc + 1]] = input[in_idx++];
					pc += 2;
				}
				break;
			case OUT:
				outval = get_parm(mem[pc + 1], mode[0], mem);
				pc += 2;
				break;
			case JNE:
				if (get_parm(mem[pc + 1], mode[0], mem) != 0)
					pc = get_parm(mem[pc + 2], mode[1], mem);
				else
					pc += 3;
				break;
			case JEQ:
				if (get_parm(mem[pc + 1], mode[0], mem) == 0)
					pc = get_parm(mem[pc + 2], mode[1], mem);
				else
					pc += 3;
				break;
			case LT:
				mem[mem[pc + 3]] = get_parm(mem[pc + 1], mode[0], mem) < get_parm(mem[pc + 2], mode[1], mem) ? 1 : 0;
				pc += 4;
				break;
			case EQ:
				mem[mem[pc + 3]] = get_parm(mem[pc + 1], mode[0], mem) == get_parm(mem[pc + 2], mode[1], mem) ? 1 : 0;
				pc += 4;
				break;
			default:
				fprintf(stderr, "Invalid opcode %d @ %d\n", opcode, pc);
				exit(1);
		}
	}

	free(mem);
	return outval;
}

int run_amps(int* prog, int prog_sz, int* phases) {
	int input[2];
	input[1] = 0; // input into A
	for (int amp = 0; amp < NR_AMPS; ++amp) {
		input[0] = phases[amp];
		input[1] = run_program(prog, prog_sz, input);
	}
	return input[1];
}

void print_phases(int* phases) {
	for (int ii = 0; ii < NR_AMPS; ++ii)
		printf("%d", phases[ii]);
	printf("\n");
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s progfile\n", argv[0]);
		return 1;
	}

	int mem[MEMSZ];
	int mem_used = read_prog_from_file(mem, argv[1]);

	int phases[NR_AMPS];
	for (int ii = 0; ii < NR_AMPS; ++ii)
		phases[ii] = ii;

	int max_out = 0;
	do {
		int out = run_amps(mem, mem_used, phases);
		max_out = out > max_out ? out : max_out;
	} while (next_perm(phases, NR_AMPS));
	printf("%d\n", max_out);

	return 0;
}
