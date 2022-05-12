#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct lettercount {
	char letter;
	int count;
};

int cmp_lettercount(const void* a, const void* b) {
	// order count large to small
	// order letter small to large
	if (((struct lettercount*)b)->count == ((struct lettercount*)a)->count)
		return ((struct lettercount*)a)->letter - ((struct lettercount*)b)->letter;
	return ((struct lettercount*)b)->count - ((struct lettercount*)a)->count;
}

int check_room(char* line) {
	int id = 0;
	int ii;
	struct lettercount lc[26];
	for (ii = 0; ii < 26; ++ii) {
		lc[ii].letter = 'a' + ii;
		lc[ii].count = 0;
	}
	// encrypted name
	while (*line < '0' || *line > '9') {
		if (*line >= 'a' && *line <= 'z')
			++lc[*line - 'a'].count;
		++line;
	}
	// read sector ID
	while (*line >= '0' && *line <= '9') {
		id = id * 10 + (*line - '0');
		++line;
	}
	++line; // skip [
	// check checksum
	qsort(lc, 26, sizeof(struct lettercount), cmp_lettercount);
	for (ii = 0; ii < 5; ++ii) {
		if (lc[ii].letter != *line++)
			return 0;
	}
	return id;
}

int main(int argc, char* argv[]) {
	int count = 0;

	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		count += check_room(line);
	}
	free(line);
	printf("%d\n", count);
	return 0;
}
