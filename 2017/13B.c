#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define MAXDEPTH 128

bool read_int(char** pLine, int* x) {
	int num = 0;
	char* line = *pLine;
	while (*line && !isdigit(*line))
		++line;
	if (!isdigit(*line))
		return false;
	while (isdigit(*line)) {
		num = num * 10 + *line - '0';
		++line;
	}
	*x = num;
	*pLine = line;
	return true;
}

int main(int argc, char* argv[]) {
	int period[MAXDEPTH + 1] = {0};

	char *line = NULL;
	size_t len = 0;
	int maxdepth = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		int depth, range;
		read_int(&l, &depth);
		read_int(&l, &range);
		period[depth] = 2*(range - 1);
		maxdepth = depth > maxdepth ? depth : maxdepth;
	}
	// brute force non-intelligent algo, but still fast enough...
	// We could speed up by factor of 2 by finding this t0 needs to
	// be either od  or even (looking for period 2 in the table).
	// Looking for period 6, we can get more info. This speeds up by
	// factor 3, etc. But since I already solved in acceptable time,
	// I decided to move on...
	// Code simplicity preferred here.
	int t0 = 0;
	bool found = false;
	while (!found) {
		bool caught = false;
		for (int d = 0; !caught && d <= maxdepth; ++d) {
			caught = period[d] > 0 && (t0 + d) % period[d] == 0;
		}
		found = !caught;
		++t0;
	}
	if (found)
		printf("%d\n", t0 - 1);

	free(line);
	return 0;
}
