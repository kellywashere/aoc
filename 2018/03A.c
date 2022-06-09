#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GRIDSIZE 1000

int read_int(char** pLine) {
	int num = 0;
	char* line = *pLine;
	while (*line && !(*line >= '0' && *line <= '9'))
		++line;
	while (*line >= '0' && *line <= '9') {
		num = num * 10 + *line - '0';
		++line;
	}
	*pLine = line;
	return num;
}

int main(int argc, char* argv[]) {
	int* grid = calloc(GRIDSIZE * GRIDSIZE, sizeof(int));
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, stdin) != -1) {
		char* l = line;
		/* int id = */ read_int(&l);
		int x = read_int(&l);
		int y = read_int(&l);
		int w = read_int(&l);
		int h = read_int(&l);
		for (int yy = y; yy < y + h; ++yy) {
			int idx = yy * GRIDSIZE + x;
			for (int ww = 0; ww < w; ++ww)
				++grid[idx++];
		}
	}
	free(line);

	int count = 0;
	for (int ii = 0; ii < GRIDSIZE * GRIDSIZE; ++ii)
		count += grid[ii] >= 2 ? 1 : 0;
	printf("%d\n", count);

	free(grid);
	return 0;
}
