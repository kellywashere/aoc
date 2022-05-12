#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int read_int(char* line, int* pIdx) {
	int num = 0;
	while (line[*pIdx] >= '0' && line[*pIdx] <= '9') {
		num = num * 10 + line[*pIdx] - '0';
		++*pIdx;
	}
	return num;
}

int main(int argc, char* argv[]) {
	int count = 0;
	char *line = NULL;
	size_t len = 0;
	if (getline(&line, &len, stdin) != -1) {
		int ii = 0;
		while (line[ii] && line[ii] != '\n') {
			if (line[ii] == '(') {
				++ii; // skip '('
				int nrChars = read_int(line, &ii);
				++ii; // skip 'x'
				int nrReps = read_int(line, &ii);
				++ii; // skip ')'
				count += nrChars * nrReps;
				ii += nrChars; // skip the repeated chars
			}
			else {
				++count;
				++ii;
			}
		}
	}
	printf("%d\n", count);
	free(line);
	return 0;
}
