#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MEMSZ 1024

bool read_int(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

int run_program(int* mem_orig, int noun, int verb) {
	int mem[MEMSZ];
	memcpy(mem, mem_orig, MEMSZ * sizeof(int));

	mem[1] = noun;
	mem[2] = verb;

	int pc = 0;
	while (mem[pc] != 99) {
		int opcode = mem[pc];
		if (opcode == 1)
			mem[mem[pc + 3]] = mem[mem[pc + 1]] + mem[mem[pc + 2]];
		else if (opcode == 2)
			mem[mem[pc + 3]] = mem[mem[pc + 1]] * mem[mem[pc + 2]];
		else
			fprintf(stderr, "Invalid opcode %d @ %d\n", opcode, pc);
		pc += 4;
	}
	return mem[0];
}

int main(int argc, char* argv[]) {
	int mem[MEMSZ];

	char *line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);

	char* l = line;
	int mem_used = 0;
	int x;
	while (read_int(&l, &x))
		mem[mem_used++] = x;
	free(line);

	for (int nv = 0; nv <= 9999; ++nv) {
		if (run_program(mem, nv / 100, nv % 100) == 19690720) {
			printf("%d\n", nv);
			break;
		}
	}
	return 0;
}
