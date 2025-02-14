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

void emulate_monad(int* regs) {
	int A[] = { 1,  1,  1,  1,  26,  1,  1, 26,  1, 26, 26,  26, 26, 26};
	int B[] = {15, 11, 10, 12, -11, 11, 14, -6, 10, -6, -6, -16, -4, -2};
	int C[] = { 9,  1, 11,  3,  10,  5,  0,  7,  9, 15,  4,  10,  4,  9};
	// regs: w,x,y,z
	for (int ii = 0; ii < 14; ++ii) {
		regs[0] = fgetc(stdin) - '0';
		regs[1] = (regs[3] % 26) + B[ii];
		regs[3] /= A[ii];
		if (regs[0] != regs[1])
			regs[3] = 26 * regs[3] + regs[0] + C[ii];
	}
}

void print_regs(int* regs) {
	for (int ii = 0; ii < 4; ++ii)
		printf("%c = %d\n", 'w' + ii, regs[ii]);
}

int main(int argc, char* argv[]) {
	int regs[4] = {0};
	emulate_monad(regs);
	print_regs(regs);
	return 0;
}
