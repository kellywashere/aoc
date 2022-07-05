#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

// max nodes: (26 letters & 10 digits)^3
#define NR_CHARS  36
#define MAX_NODES (NR_CHARS*NR_CHARS*NR_CHARS)

int get_idx(char** pLine) {
	char* l = *pLine;
	int idx = 0;
	while (isupper(*l) || isdigit(*l)) {
		idx *= NR_CHARS;
		if (isdigit(*l))
			idx += (*l) - '0';
		else
			idx += (*l) - 'A' + 10;
		++l;
	}
	*pLine = l;
	return idx;
}

int main(int argc, char* argv[]) {
	int* orbits = malloc(MAX_NODES * sizeof(int));
	for (int ii = 0; ii < MAX_NODES; ++ii)
		orbits[ii] = -1; // ii does not orbit (yet?)

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int obj1 = get_idx(&l);
		++l;
		int obj2 = get_idx(&l);
		// obj2 orbits obj1
		orbits[obj2] = obj1;
	}

	// count
	int count = 0;
	for (int ii = 0; ii < MAX_NODES; ++ii) {
		if (orbits[ii] >= 0) {
			int idx = ii;
			do {
				++count;
				idx = orbits[idx];
			} while (orbits[idx] >= 0);
		}
	}
	printf("%d\n", count);

	free(line);
	free(orbits);
	return 0;
}
