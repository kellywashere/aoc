#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define SIZE 20

bool empty_line(const char* line) {
	while (*line && isspace(*line))
		++line;
	return *line == '\0';
}

int read_int(char** pLine) {
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
	*pLine = line;
	return num;
}

int main(int argc, char* argv[]) {
	char *line = NULL;
	size_t len = 0;
	int x[3];
	bool cubes[SIZE * SIZE * SIZE] = {0};
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		for (int ii = 0; ii < 3; ++ii)
			x[ii] = read_int(&l);
		cubes[x[2] * SIZE * SIZE + x[1] * SIZE + x[0]] = true;
	}

	int count = 0;
	for (x[2] = 0; x[2] < SIZE; ++x[2]) {
		for (x[1] = 0; x[1] < SIZE; ++x[1]) {
			for (x[0] = 0; x[0] < SIZE; ++x[0]) {
				if (cubes[x[2] * SIZE * SIZE + x[1] * SIZE + x[0]]) {
					int n = 0; // neighbors
					n += x[2] > 0        && cubes[(x[2] - 1) * SIZE * SIZE + x[1] * SIZE + x[0]] ? 1 : 0;
					n += x[2] < SIZE - 1 && cubes[(x[2] + 1) * SIZE * SIZE + x[1] * SIZE + x[0]] ? 1 : 0;
					n += x[1] > 0        && cubes[x[2] * SIZE * SIZE + (x[1] - 1) * SIZE + x[0]] ? 1 : 0;
					n += x[1] < SIZE - 1 && cubes[x[2] * SIZE * SIZE + (x[1] + 1) * SIZE + x[0]] ? 1 : 0;
					n += x[0] > 0        && cubes[x[2] * SIZE * SIZE + x[1] * SIZE + x[0] - 1] ? 1 : 0;
					n += x[0] < SIZE - 1 && cubes[x[2] * SIZE * SIZE + x[1] * SIZE + x[0] + 1] ? 1 : 0;
					count += 6 - n;
				}
			}
		}
	}
	printf("%d\n", count);

	free(line);
	return 0;
}
