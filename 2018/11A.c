#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GRID_WIDTH 300

int calc_power_level(int x, int y, int serial) {
	// NOTE: x and y are ZERO based here, but 1-based in puzzle
	int rid = x + 1 + 10;
	int p = (rid * (y + 1) + serial) * rid;
	return (p / 100) % 10 - 5;
}

int main(int argc, char* argv[]) {
	int serial = 8199;
	if (argc > 1)
		serial = atoi(argv[1]);
	// fill grid
	int grid[GRID_WIDTH * GRID_WIDTH];
	for (int y = 0; y < GRID_WIDTH; ++y)
		for (int x = 0; x < GRID_WIDTH; ++x)
			grid[y * GRID_WIDTH + x] = calc_power_level(x, y, serial);
	// find largest 3x3 square
	int maxp = 0;
	int maxx = 0;
	int maxy = 0;
	for (int y = 0; y < GRID_WIDTH - 2; ++y) {
		for (int x = 0; x < GRID_WIDTH - 2; ++x) {
			int tot = 0;
			for (int row = 0; row <= 2; ++row) {
				int idx = (y + row) * GRID_WIDTH + x;
				tot += grid[idx] + grid[idx + 1] + grid[idx + 2];
			}
			if (tot > maxp) {
				maxp = tot;
				maxx = x;
				maxy = y;
			}
		}
	}
	printf("%d,%d\n", maxx + 1, maxy + 1);

	return 0;
}
