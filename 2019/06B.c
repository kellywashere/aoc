#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

// max nodes: (26 letters & 10 digits)^3
#define NR_CHARS  36
#define MAX_NODES (NR_CHARS*NR_CHARS*NR_CHARS)

#define MAX_PATHLEN 512

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

	// path from YOU to root
	char* l = "YOU";
	int idx = get_idx(&l);
	int path_you[1024];
	int path_you_len = 0;
	do {
		path_you[path_you_len++] = idx;
		idx = orbits[idx];
	} while (idx >= 0);
	// path from YOU to root
	l = "SAN";
	idx = get_idx(&l);
	int path_san[1024];
	int path_san_len = 0;
	do {
		path_san[path_san_len++] = idx;
		idx = orbits[idx];
	} while (idx >= 0);
	// find common part
	while (path_you[path_you_len - 1] == path_san[path_san_len - 1]) {
		--path_you_len;
		--path_san_len;
	}
	printf("%d\n", path_you_len + path_san_len - 2);

	free(line);
	free(orbits);
	return 0;
}
