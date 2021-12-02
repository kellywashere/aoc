#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define GRIDSIZE 300

const char* dirstr = "^v<>";
const int dx[] = {0, 0, -1, 1};
const int dy[] = {-1, 1, 0, 0};

void mark(bool* grid, int x, int y, int* nr_visited) {
	int idx = x + y * GRIDSIZE;
	if (idx >= GRIDSIZE * GRIDSIZE)
		fprintf(stderr, "Out of the grid\n");
	else if (!grid[idx]) {
		grid[idx] = true;
		++(*nr_visited);
	}
}

int main(int argc, char* argv[]) {
	bool* grid = calloc(GRIDSIZE * GRIDSIZE, sizeof(bool));
	int x[2];
	int y[2];
	x[0] = x[1] = y[0] = y[1] = GRIDSIZE/2;
	int nr_visited = 0;
	mark(grid, x[0], y[0], &nr_visited);
	char c;
	int mover = 0;
	while ( (c = fgetc(stdin)) != EOF) {
		char* cloc = strchr(dirstr, c);
		if (cloc == NULL)
			continue;
		int didx = cloc - dirstr;
		x[mover] += dx[didx];
		y[mover] += dy[didx];
		mark(grid, x[mover], y[mover], &nr_visited);
		mover = 1 - mover;
	}
	printf("%d\n", nr_visited);
	return 0;
}
