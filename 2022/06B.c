#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define LEN 14

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int lettercount[26] = {0};

	getline(&line, &len, stdin);

	// init lettercount
	for (int ii = 0; ii < LEN; ++ii)
		++lettercount[line[ii] - 'a'];

	int idx = LEN - 1;
	int idx_found = 0;
	while (!idx_found && isalpha(line[idx])) {
		bool alldifferent = true;
		for (int ii = 0; alldifferent && ii < 26; ++ii)
			alldifferent = alldifferent && lettercount[ii] < 2;
		++idx;
		if (alldifferent)
			idx_found = idx;
		if (isalpha(line[idx])) {
			--lettercount[line[idx - LEN] - 'a'];
			++lettercount[line[idx] - 'a'];
		}
	}
	free(line);

	printf("%d\n", idx_found);
	return 0;
}
