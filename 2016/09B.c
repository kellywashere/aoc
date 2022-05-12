#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

int read_int(char* line, int* pIdx) {
	int num = 0;
	while (line[*pIdx] >= '0' && line[*pIdx] <= '9') {
		num = num * 10 + line[*pIdx] - '0';
		++*pIdx;
	}
	return num;
}

uint64_t process_section(char* line, int idx, int len) {
	// processed a substring of length len, starting at idx
	// returns how many characters that piece produces
	// recursive!
	uint64_t count = 0;
	int ii = idx;
	while (ii < idx + len) {
		if (line[ii] == '(') {
			++ii; // skip '('
			int nrChars = read_int(line, &ii);
			++ii; // skip 'x'
			int nrReps = read_int(line, &ii);
			++ii; // skip ')'
			count += nrReps * process_section(line, ii, nrChars);
			ii += nrChars;
		}
		else {
			++count;
			++ii;
		}
	}
	return count;
}

int main(int argc, char* argv[]) {
	uint64_t count = 0;
	char *line = NULL;
	size_t len = 0;
	if (getline(&line, &len, stdin) != -1) {
		int len = 0;
		while (line[len] && line[len] != '\n')
			++len;
		count = process_section(line, 0, len);
	}
	printf("%" PRIu64 "\n", count);
	free(line);
	return 0;
}
