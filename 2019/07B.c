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

struct amplifier {
	int  prog[MEMSZ];
	int  pc;
	int  phase;
	bool phase_read;
	int  input;
	int  out;
};

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


////////////////// DISASM //////////////////////////

void print_parm(int operand, int mode) {
	if (mode == 0)
		printf("[%d]", operand);
	else
		printf("%d", operand);
}

void disasm(int* mem, int mem_used) {
	int pc = 0;
	while (pc < mem_used) {
		printf("%4d: ", pc);
		int m = mem[pc] / 100;
		int opcode = mem[pc] - 100 * m;
		int mode[3];
		for (int ii = 0; ii < 3; ++ii) {
			mode[ii] = m % 10;
			m /= 10;
		}

		switch (opcode) {
			case ADD:
				print_parm(mem[pc + 3], mode[2]);
				printf(" = ");
				print_parm(mem[pc + 1], mode[0]);
				printf(" + ");
				print_parm(mem[pc + 2], mode[1]);
				printf("\n");
				pc += 4;
				break;
			case MUL:
				print_parm(mem[pc + 3], mode[2]);
				printf(" = ");
				print_parm(mem[pc + 1], mode[0]);
				printf(" * ");
				print_parm(mem[pc + 2], mode[1]);
				printf("\n");
				pc += 4;
				break;
			case INP:
				print_parm(mem[pc + 1], mode[0]);
				printf(" <- in\n");
				pc += 2;
				break;
			case OUT:
				print_parm(mem[pc + 1], mode[0]);
				printf(" -> out\n");
				pc += 2;
				break;
			case JNE:
				printf("if (");
				print_parm(mem[pc + 1], mode[0]);
				printf(" != 0) goto ");
				print_parm(mem[pc + 2], mode[1]);
				printf("\n");
				pc += 3;
				break;
			case JEQ:
				printf("if (");
				print_parm(mem[pc + 1], mode[0]);
				printf(" == 0) goto ");
				print_parm(mem[pc + 2], mode[1]);
				printf("\n");
				pc += 3;
				break;
			case LT:
				print_parm(mem[pc + 3], mode[2]);
				printf(" = (");
				print_parm(mem[pc + 1], mode[0]);
				printf(" < ");
				print_parm(mem[pc + 2], mode[1]);
				printf(") ? 1 : 0\n");
				pc += 4;
				break;
			case EQ:
				print_parm(mem[pc + 3], mode[2]);
				printf(" = (");
				print_parm(mem[pc + 1], mode[0]);
				printf(" == ");
				print_parm(mem[pc + 2], mode[1]);
				printf(") ? 1 : 0\n");
				pc += 4;
				break;
			case HLT:
				printf("HALT\n");
				pc += 1;
				break;
			default:
				printf(" -- #%d\n", mem[pc]);
				pc += 1;
		}
	}
}


int run_program(struct amplifier* amp) {
	int outval = 0;
	int* mem = amp->prog;
	int pc = amp->pc;
	bool input_consumed = false; // error detection
	while (mem[pc] != HLT) {
		int outval = 0;
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
				int in = 0;
				if (!amp->phase_read) {
					in = amp->phase;
					amp->phase_read = true;
				}
				else if (!input_consumed)
					in = amp->input;
				else
					fprintf(stderr, "More inputs asked than expected\n");
				mem[mem[pc + 1]] = in;
				pc += 2;
				break;
			case OUT:
				outval = get_parm(mem[pc + 1], mode[0], mem);
				pc += 2;
				amp->pc = pc;
				return outval;
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
	return outval;
}

void amp_init(struct amplifier* amp, int* prog, int phase) {
	memcpy(amp->prog, prog, MEMSZ * sizeof(int));
	amp->pc = 0;
	amp->phase = phase;
	amp->phase_read = false;
}

bool amp_is_halted(struct amplifier* amp) {
	return amp->prog[amp->pc] == HLT;
}

int run_amps(int* prog, struct amplifier* amps, int* phases) {
	for (int amp = 0; amp < NR_AMPS; ++amp)
		amp_init(&amps[amp], prog, phases[amp]);
	amps[0].input = 0;
	int out = 0;
	while (!amp_is_halted(&amps[NR_AMPS - 1])) {
		for (int amp = 0; amp < NR_AMPS; ++amp) {
			out = run_program(&amps[amp]);
			int next_amp = (amp + 1) % NR_AMPS;
			amps[next_amp].input = out;
		}
	}
	return out;
}

void print_phases(int* phases) {
	for (int ii = 0; ii < NR_AMPS; ++ii)
		printf("%d", phases[ii]);
	printf("\n");
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s progfile\n", argv[0]);
		printf("     : %s -d progfile    for disassembly\n", argv[0]);
		return 1;
	}
	int prg_idx = 1;
	bool do_disassembly = false;
	if (argc >= 3 && !strncmp(argv[1], "-d", 2)) {
		++prg_idx;
		do_disassembly = true;
	}

	int mem[MEMSZ];
	int mem_used = read_prog_from_file(mem, argv[prg_idx]);
	
	if (do_disassembly) {
		disasm(mem, mem_used);
		return 0;
	}

	struct amplifier amps[NR_AMPS];

	int phases[NR_AMPS];
	for (int ii = 0; ii < NR_AMPS; ++ii)
		phases[ii] = ii + 5;

	int max_out = 0;
	do {
		int out = run_amps(mem, amps, phases);
		max_out = out > max_out ? out : max_out;
	} while (next_perm(phases, NR_AMPS));
	printf("%d\n", max_out);

	return 0;
}
