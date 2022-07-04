#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

	// 1202
	mem[1] = 12;
	mem[2] = 02;

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
	printf("%d\n", mem[0]);

	return 0;
}
