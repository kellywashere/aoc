#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int nrbits = 0;
	int* counters;

	while (getline(&line, &len, stdin) != -1) {
		if (!nrbits) {
			char* s = line;
			while (*s == '0' || *s == '1')
				++s;
			nrbits = s - line;
			counters = calloc(nrbits, sizeof(int));
		}
		for (int ii = 0; ii < nrbits; ++ii)
			counters[ii] += line[ii] == '1' ? 1 : -1;
	}
	int gamma   = 0;
	int epsilon = 0;
	for (int ii = 0; ii < nrbits; ++ii) {
		if (counters[ii] == 0)
			fprintf(stderr, "Undefined gamma & epsilon bit at pos %d\n", ii);
		gamma = 2*gamma + (counters[ii] > 0 ? 1 : 0);
		epsilon = 2*epsilon + (counters[ii] < 0 ? 1 : 0);
	}
	printf("%d\n", gamma * epsilon);
	free(line);
	free(counters);
	return 0;
}
