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
	int x = GRIDSIZE/2;
	int y = GRIDSIZE/2;
	int nr_visited = 0;
	mark(grid, x, y, &nr_visited);
	char c;
	while ( (c = fgetc(stdin)) != EOF) {
		char* cloc = strchr(dirstr, c);
		if (cloc == NULL)
			continue;
		int didx = cloc - dirstr;
		x += dx[didx];
		y += dy[didx];
		mark(grid, x, y, &nr_visited);
	}
	printf("%d\n", nr_visited);
	return 0;
}
