#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool empty_line(const char* line) {
	while (*line) {
		if (*line != ' ' && *line != '\t' && *line != '\n')
			return false;
		++line;
	}
	return true;
}

void emulate_line(char* line, int* regs) {
	// regs: w,x,y,z
	char instr[4];
	int reg1idx = -1;
	int value = 0;

	char* t = strtok(line, " \t");
	strncpy(instr, t, 3);
	instr[3] = 0;

	t = strtok(NULL, " \t");
	reg1idx = *t - 'w';

	if (!strcmp(instr, "inp")) {
		regs[reg1idx] = fgetc(stdin) - '0';
	}
	else {
		t = strtok(NULL, " \t");
		if (t) {
			if (*t >= 'w' && *t <= 'z')
				value = regs[*t - 'w'];
			else
				value = atoi(t);
		}
		if (!strcmp(instr, "add"))
			regs[reg1idx] += value;
		else if (!strcmp(instr, "mul"))
			regs[reg1idx] *= value;
		else if (!strcmp(instr, "div"))
			regs[reg1idx] /= value;
		else if (!strcmp(instr, "mod"))
			regs[reg1idx] %= value;
		else if (!strcmp(instr, "eql"))
			regs[reg1idx] = regs[reg1idx] == value ? 1 : 0;
	}
}

void print_regs(int* regs) {
	for (int ii = 0; ii < 4; ++ii)
		printf("%c = %d\n", 'w' + ii, regs[ii]);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s filename\n", argv[0]);
		return 1;
	}
	FILE* fp = fopen(argv[1], "r");
	if (!fp) {
		fprintf(stderr, "Could not open %s for reading\n", argv[1]);
		return 2;
	}

	int regs[4] = {0};
	char *line = NULL;

	size_t len = 0;
	while (getline(&line, &len, fp) != -1) {
		if (empty_line(line))
			continue;
		emulate_line(line, regs);
	}
	print_regs(regs);
	free(line);
	return 0;
}
