#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MEMSZ 1024

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

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s progfile\n", argv[0]);
		return 1;
	}

	int mem[MEMSZ];
	/*int mem_used =*/read_prog_from_file(mem, argv[1]);

	int pc = 0;
	while (mem[pc] != HLT) {
		int x;
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
				printf("Value: ");
				scanf("%d", &x);
				mem[mem[pc + 1]] = x;
				pc += 2;
				break;
			case OUT:
				printf("%d\n", get_parm(mem[pc + 1], mode[0], mem));
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

	return 0;
}
