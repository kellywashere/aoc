#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

#define MAX_NRS_PER_ROW 128

int checksum(char* l) {
	int x[MAX_NRS_PER_ROW];
	int n = 0; // nr of integers read
	for ( ; read_int(&l, &x[n]); ++n)
		;
	for (int i1 = 0; i1 < n - 1; ++i1) {
		for (int i2 = i1 + 1; i2 < n; ++i2) {
			if (x[i1] % x[i2] == 0)
				return x[i1]/x[i2];
			if (x[i2] % x[i1] == 0)
				return x[i2]/x[i1];
		}
	}
	return 0;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int sum = 0;
	while (getline(&line, &len, stdin) != -1) {
		sum += checksum(line);
	}
	printf("%d\n", sum);
	free(line);
	return 0;
}
