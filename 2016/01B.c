#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// x = -GRIDSIZE..GRIDSIZE
// y = -GRIDSIZE..GRIDSIZE
#define GRIDSIZE 250

bool mark_visited(bool* grid, int x, int y) {
	if (x < -GRIDSIZE || x > GRIDSIZE || y < -GRIDSIZE || y > GRIDSIZE) {
		fprintf(stderr, "Oops, moving off the grid\n");
		exit(1);
	}
	x += GRIDSIZE;
	y += GRIDSIZE;
	int idx = y*(2*GRIDSIZE + 1) + x;
	if (grid[idx])
		return true;
	grid[idx] = true;
	return false;
}

int main(int argc, char* argv[]) {
	int dir; // 1: left, -1: right
	int num = 0;
	int x = 0;
	int y = 0;
	//face North
	int dx = 0;
	int dy = 1;
	char c;
	bool* grid = calloc((2*GRIDSIZE + 1)*(2*GRIDSIZE + 1), sizeof(bool));

	mark_visited(grid, x, y);
	bool found_location = false;

	while (!found_location && (c = fgetc(stdin)) != EOF) {
		if (c == 'L')
			dir = 1;
		else if (c == 'R')
			dir = -1;
		else if (c >= '0' && c <= '9')
			num = num * 10 + c - '0';
		else if (num > 0) { // apply instruction
			// rotate
			int dx_next = -dir * dy;
			dy = dir * dx;
			dx = dx_next;
			// walk in direction [dx, dy]
			while (num && !found_location) {
				x += dx;
				y += dy;
				if (mark_visited(grid, x, y))
					found_location = true;
				--num;
			}
		}
	}
	// abs
	x = x < 0 ? -x : x;
	y = y < 0 ? -y : y;
	printf("%d\n", x + y);
	free(grid);
	return 0;
}
